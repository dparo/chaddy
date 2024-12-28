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
    curl \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory in the container
WORKDIR /app

# Copy the local C source code and CMakeLists.txt to the container
COPY . /app

ARG VCPKG_ROOT=/app/vcpkg
# Create a build directory and run cmake to build the project
RUN cmake -B build -S ./ --preset release

# Stage 2: Run the C program

# FROM alpine:3.21
FROM ubuntu:24.04


# Set the working directory for the runtime image
WORKDIR /app

# Copy the compiled executable from the build stage
COPY --from=builder /app/build/src/chaddy /app/chaddy

# Expose port 3000 to the outside world
EXPOSE 3000

# Set the default command to run the executable
CMD ["/app/chaddy", "-p", "3000"]

