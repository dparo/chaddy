#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <fnmatch.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
#define MAX_EVENTS 10

const char *ignored_dirs[] = {".git", "node_modules", "__pycache__", "build", NULL};
char **gitignore_patterns = NULL;
size_t gitignore_count = 0;

int is_ignored(const char *path, const char *name) {
    for (int i = 0; ignored_dirs[i] != NULL; i++) {
        if (strcmp(name, ignored_dirs[i]) == 0) {
            return 1;
        }
    }
    char full_path[PATH_MAX];
    snprintf(full_path, PATH_MAX, "%s/%s", path, name);
    for (size_t i = 0; i < gitignore_count; i++) {
        if (fnmatch(gitignore_patterns[i], full_path, 0) == 0 || fnmatch(gitignore_patterns[i], name, 0) == 0) {
            return 1;
        }
    }
    return 0;
}

void load_gitignore(const char *path) {
    char gitignore_path[PATH_MAX];
    snprintf(gitignore_path, PATH_MAX, "%s/.gitignore", path);

    FILE *file = fopen(gitignore_path, "r");
    if (!file) return;

    char line[PATH_MAX];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) > 0 && line[0] != '#') {
            gitignore_patterns = realloc(gitignore_patterns, sizeof(char *) * (gitignore_count + 1));
            char pattern[PATH_MAX];
            snprintf(pattern, PATH_MAX, "%s/%s", path, line);
            gitignore_patterns[gitignore_count] = strdup(pattern);
            gitignore_count++;
        }
    }
    fclose(file);
}

void watch_directory(int inotify_fd, const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    load_gitignore(path);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0 &&
            !is_ignored(path, entry->d_name)) {
            char subdir[PATH_MAX];
            snprintf(subdir, PATH_MAX, "%s/%s", path, entry->d_name);
            watch_directory(inotify_fd, subdir);
        }
    }

    int wd = inotify_add_watch(inotify_fd, path, IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_MOVED_TO);
    if (wd == -1) {
        perror("inotify_add_watch");
    }
    closedir(dir);
}

int has_valid_extension(const char *filename, char **extensions, int ext_count) {
    for (int i = 0; i < ext_count; i++) {
        size_t len_filename = strlen(filename);
        size_t len_extension = strlen(extensions[i]);
        if (len_filename >= len_extension &&
            strcmp(filename + len_filename - len_extension, extensions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <directory> <extension1> [<extension2> ...] <command>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *watch_dir = argv[1];
    char **extensions = &argv[2];
    int ext_count = argc - 3;
    const char *command = argv[argc - 1];

    int inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd < 0) {
        perror("inotify_init");
        return EXIT_FAILURE;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        return EXIT_FAILURE;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = inotify_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &ev) < 0) {
        perror("epoll_ctl");
        return EXIT_FAILURE;
    }

    watch_directory(inotify_fd, watch_dir);

    struct epoll_event events[MAX_EVENTS];
    char buffer[BUF_LEN];
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == inotify_fd) {
                ssize_t length = read(inotify_fd, buffer, BUF_LEN);
                if (length < 0) {
                    perror("read");
                    continue;
                }

                int j = 0;
                while (j < length) {
                    struct inotify_event *event = (struct inotify_event *)&buffer[j];
                    if (event->mask & IN_CREATE && (event->mask & IN_ISDIR)) {
                        char new_dir[PATH_MAX];
                        snprintf(new_dir, PATH_MAX, "%s/%s", watch_dir, event->name);
                        watch_directory(inotify_fd, new_dir);
                    }
                    if (event->len && has_valid_extension(event->name, extensions, ext_count) && !is_ignored(watch_dir, event->name)) {
                        printf("File event detected: %s\n", event->name);
                        system(command);
                    }
                    j += EVENT_SIZE + event->len;
                }
            }
        }
    }

    for (size_t i = 0; i < gitignore_count; i++) {
        free(gitignore_patterns[i]);
    }
    free(gitignore_patterns);
    close(inotify_fd);
    close(epoll_fd);
    return EXIT_SUCCESS;
}

