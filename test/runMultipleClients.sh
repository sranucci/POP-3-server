#!/bin/bash

# Array to store child process IDs
child_pids=()

# Function to clean up child processes
cleanup() {
  for pid in "${child_pids[@]}"; do
    echo "Killing child PID: $pid"
    kill "$pid" >/dev/null 2>&1
  done
  exit 0
}

# Register the signal handler
trap cleanup SIGINT SIGTERM

# Check if the number of clients argument is provided
if [ $# -eq 0 ]; then
  echo "Usage: $0 <number_of_clients>"
  exit 1
fi

# Retrieve the number of clients from the command line argument
number_of_clients=$1

# Run the desired number of client instances simultaneously
for ((i=1; i<=$number_of_clients; i++)); do
  python3 client.py &
  child_pids+=("$!")
done

# Wait for all client instances to finish
wait
