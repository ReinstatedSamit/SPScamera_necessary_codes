import ttn_storage_api  # Assuming you have the `ttn_storage_api.py` in the same directory
import base64
# Set your TTN V3 API key (replace with your actual key)
key = ""

# Set the TTN application name (replace 'my-app-name' with your actual app name)
app_name = "spscamera"

# Pull data from TTN for the last 24 hours
data = ttn_storage_api.sensor_pull_storage(app_name, key, "24h")

# Display the number of records
print("Number of records:", len(data))
# If there is data, let's inspect the raw response and decode frm_payload

image_data=""
# If there is data, let's inspect the raw response and decode frm_payload only for f_port == 6
if len(data) > 0:
    for index, record in enumerate(data):
        uplink_message = record.get('result', {}).get('uplink_message', None)

        if uplink_message:
            # Check if f_port is 6
            f_port = uplink_message.get('f_port', None)
            if f_port == 6:
                # Extract received_at timestamp
                time = uplink_message.get('received_at', 'N/A')

                # Extract device ID
                entity_id = record.get('result', {}).get('end_device_ids', {}).get('device_id', 'N/A')

                # Check if frm_payload is present (base64 encoded)
                frm_payload = uplink_message.get('frm_payload', None)

                if frm_payload:
                    # Decode the base64 encoded payload
                    decoded_payload = base64.b64decode(frm_payload).decode('utf-8', errors='ignore')
                    image_data += decoded_payload
                    print(f"Time: {time}")
                    print(f"Entity ID: {entity_id}")
                    print(f"Decoded Payload: {decoded_payload}")
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

print(image_data
