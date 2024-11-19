import ttn_storage_api  # Assuming you have the `ttn_storage_api.py` in the same directory
import base64
from datetime import datetime
import binascii
import matplotlib.pyplot as plt
from PIL import Image
# Set your TTN V3 API key (replace with your actual key)
key = ""

# Set the TTN application name (replace 'my-app-name' with your actual app name)
app_name = "spscamera"

# Pull data from TTN for the last 24 hours
data = ttn_storage_api.sensor_pull_storage(app_name, key, "24h")

# Display the number of records
print("Number of records:", len(data))
# If there is data, let's inspect the raw response and decode frm_payload
'''if len(data) > 0:
    for record in data:
        uplink_message = record.get('result', {}).get('uplink_message', None)

        if uplink_message:
            # Extract received_at timestamp
            time = uplink_message.get('received_at', 'N/A')

            # Extract device ID
            entity_id = record.get('result', {}).get('end_device_ids', {}).get('device_id', 'N/A')

            # Check if frm_payload is present (base64 encoded)
            frm_payload = uplink_message.get('frm_payload', None)

            if frm_payload:
                # Decode the base64 encoded payload
                decoded_payload = base64.b64decode(frm_payload).decode('utf-8', errors='ignore')
                print(f"Time: {time}")
                print(f"Entity ID: {entity_id}")
                print(f"Decoded Payload: {decoded_payload}")
            else:
                print(f"Time: {time}")
                print(f"Entity ID: {entity_id}")
                print("No frm_payload found.")
        else:
            print("No 'uplink_message' field found in this record.")

        # Separator between records for clarity
        print("-" * 40)
else:
    print("No data found.")'''
image_data=""
image_data_hex=""
# If there is data, let's inspect the raw response and decode frm_payload only for f_port == 6
if len(data) > 0:
    for index, record in enumerate(data):
        uplink_message = record.get('result', {}).get('uplink_message', None)

        if uplink_message:
            # Check if f_port is 6
            f_port = uplink_message.get('f_port', None)
            if f_port == 7:
                # Extract received_at timestamp
                port_num=f_port
                time = uplink_message.get('received_at', 'N/A')

                # Extract device ID
                entity_id = record.get('result', {}).get('end_device_ids', {}).get('device_id', 'N/A')

                # Check if frm_payload is present (base64 encoded)
                frm_payload = uplink_message.get('frm_payload', None)

                if frm_payload:
                    # Decode the base64 encoded payload
                    #decoded_payload = base64.b64decode(frm_payload).decode('utf-8', errors='ignore')
                    payload= base64.b64decode(frm_payload)
                    #image_data += decoded_payload
                    image_data_hex += payload.hex()
                    print(f"Time: {time}")
                    print(f"Entity ID: {entity_id}")
                    print(f"frm_payload: {frm_payload}")
                    #print(f"Decoded Payload: {decoded_payload}")
                    print (f"hex_payload : {payload.hex()}")
                else:
                    print(f"Time: {time}")
                    print(f"Entity ID: {entity_id}")
                    print("No frm_payload found.")
            else:
                print(f"Skipping record with f_port: {f_port}")
        else:
            print("No 'uplink_message' field found in this record.")

        # Separator between records for clarity
        print("-" * 40)
else:
    print("No data found.")

#print(image_data)
print(f"Hex Data: {image_data_hex}")
print(type(image_data_hex))


# Get the current date and time
current_datetime = datetime.now()

# Format the date and time as a string (optional)
formatted_datetime = current_datetime.strftime("%Y_%m_%d_%H_%M_%S")

# Print the date and time
print("Current Date and Time:", formatted_datetime)

# Open a text file in write mode
with open(f'{formatted_datetime}_image_data_hex_port_{port_num}.txt', 'w') as file:
    # Write the hex data to the file
    file.write(f"Hex Data: {image_data_hex}")

# Optionally, print a message to confirm data is saved
print("Hex data has been saved to image_data_hex.txt")

print(len(image_data_hex))
# Remove any spaces and newline characters
'''
image_data_hex=image_data_hex.replace(' ', '')
image_data_hex=image_data_hex.replace('\n', '')
#image_data_hex=image_data_hex.upper()
# Find the JPEG start and end markers
start_index = image_data_hex.find("ffd8")
end_index = image_data_hex.rfind("ffd9") + 4  # +4 to include 'FFD9' itself

# Check if both markers are found
if start_index != -1 and end_index <= len(image_data_hex):
    # Truncate the data to the range between 'FFD8' and 'FFD9'
    image_data_hex = image_data_hex[start_index:end_index]
else:
    print("JPEG start or end marker not found in data.")

print(image_data_hex)
print(len(image_data_hex))



# Convert hex to binary and save as a .jpg file
image_data_hex = binascii.a2b_hex(image_data_hex)
with open('240x240_received_3secGap.jpg', 'wb') as image_file:
    image_file.write(image_data_hex)

# Display the saved image
image = Image.open('240x240_received_3secGap.jpg')
plt.imshow(image)
plt.axis('off')  # Hide axes for a cleaner display
plt.show()'''
