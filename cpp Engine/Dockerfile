FROM ubuntu:22.04

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Create build directory and build
RUN mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc)

# Expose port
EXPOSE 8080

# Run the server
CMD ["./build/server"]
