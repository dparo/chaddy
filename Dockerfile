# Stage 1: Build the C program
# FROM alpine:3.21 AS builder
FROM ubuntu:24.04 AS builder

# Install CMake and build dependencies
# RUN apk add --no-cache wget gzip patch cmake make ninja git gcc g++
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    pkg-config \
    zip \
    python3 \
    python3-pip \
    python3-venv \
    nodejs \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*;

# Install pnpm (lower dependecies requirements compared to npm)
RUN wget -qO- https://get.pnpm.io/install.sh | ENV="$HOME/.bashrc" SHELL="$(which bash)" bash -;

# Set the working directory in the container
WORKDIR /app

# Copy the local C source code and CMakeLists.txt to the container
COPY . /app

# Install NPM dependencies
RUN /root/.local/share/pnpm/pnpm install;
# Run tailwind to build the CSS asset
RUN /root/.local/share/pnpm/pnpm exec tailwindcss --input input.css -o output.css --optimize --minify;

ARG VCPKG_ROOT=/app/vcpkg

# Create a build directory and run cmake to build the project
RUN cmake -B /app/build -S ./ --preset release && cmake --build /app/build;


# Stage 2: Run the C program


FROM alpine:3.21
RUN apk add --no-cache gcompat;

# FROM ubuntu:24.04

RUN mkdir -p /app


# Set the working directory for the runtime image
WORKDIR /app

# Copy the compiled executable from the build stage
COPY --from=builder /app/build/src/chaddy /app/chaddy

# Expose port 3000 to the outside world
EXPOSE 3000

# Set the default command to run the executable
CMD ["/app/chaddy", "-p", "3000"]
