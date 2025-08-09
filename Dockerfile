# Use a single, consistent environment for building and running
FROM gcc:13

# Set the working directory
WORKDIR /app

# Copy all your project files into the container
COPY . .

# Install dependencies needed for compilation
RUN apt-get update && apt-get install -y \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Compile the project, linking the SQLite library
RUN gcc -c sqlite3.c -o sqlite3.o && \
    g++ -c database.cpp -o database.o -I. && \
    g++ -c main.cpp -o main.o -Iasio-1.34.2/include -I. && \
    g++ main.o database.o sqlite3.o -o bank_server -lpthread -lsqlite3

# Expose the server port
EXPOSE 18080

# The command to run when the container starts
CMD ["./bank_server"]