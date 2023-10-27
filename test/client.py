import poplib
import random
import time

# Server details
server = 'pampero.itba.edu.ar' # localhost if we are running the server locally or pampero.itba.edu.ar if we are running it in pampero
port = 8082 # POP server passive port
username = 'santi' # user we want to connect with
password = 'panti'

# Connect to the server
pop_conn = poplib.POP3(server, port)

# Get the server response
response = pop_conn.getwelcome()
print(response.decode())

# Login with username and password
pop_conn.user(username)
pop_conn.pass_(password)

# Display the response after login
response = pop_conn.getwelcome()
print(response.decode())

#Execute the "list()" command
response, email_list, _ = pop_conn.list()
print(response.decode())

# Decode and print the email list
for email in email_list:
    email_info = email.decode()
    print(email_info)

# Execute the "stat()" command
max_email, size = pop_conn.stat()

try:
    while True:
        time.sleep(2) # To prevent OS from generating millions of request altogether
        random_number = random.randint(1, max_email)
        try:
            response, email_lines, octets = pop_conn.retr(random_number)
            if response.startswith(b'+OK'):
                email_content = b'\n'.join(email_lines).decode()
                print(f"Random Email (#{random_number}):\n{email_content}")
                print(f"Octets {octets}")
            else:
                print("Unexpected response:", response.decode())
        except poplib.error_proto as e:
            # Extract the error message from the exception
            error_message = e.args[0].decode()
            print("Error occurred:", error_message)

except KeyboardInterrupt:
    print("Interrupted by user.")

finally:
    # Close the connection
    pop_conn.quit()