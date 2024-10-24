import requests
import json
import base64


def sensor_pull_storage(appname, accesskey, timestring, *, data_folder=None, ttn_version=3):
	url = f"https://nam1.cloud.thethings.network/api/v3/as/applications/{appname}/packages/storage/uplink_message"
	headers = {
		"Authorization": f"Bearer {accesskey}",
		"Accept": "application/json"
	}
	params = {
		"last": timestring,
		#"field_mask": "up.uplink_message.decoded_payload"
	}

	# Make the request to the TTN Storage API
	response = requests.get(url, headers=headers, params=params)

	# Check for errors in the response
	if response.status_code != 200:
		raise Exception(f"Error fetching data: {response.status_code} - {response.text}")

	# Split the response text into individual JSON objects (each JSON object is separated by a newline)
	raw_response = response.text.strip().splitlines()

	# Parse each JSON object individually and extract relevant fields
	parsed_data = []
	for raw_json in raw_response:
		try:
			parsed_record = json.loads(raw_json)
			parsed_data.append(parsed_record)

			# Extract the 'uplink_message' part
			if 'data' in parsed_record:
				uplink_message = parsed_record['data'].get('uplink_message', {})

				# Extract decoded_payload (if available)
				if 'decoded_payload' in uplink_message:
					decoded_payload = uplink_message['decoded_payload']
					print(f"Decoded Payload: {decoded_payload}")
				else:
					print("No decoded payload available.")

				# Extract Base64 encoded frm_payload (raw data)
				frm_payload = uplink_message.get('frm_payload', None)
				if frm_payload:
					decoded_raw_payload = base64.b64decode(frm_payload).decode('utf-8', errors='ignore')
					print(f"Decoded Raw Payload: {decoded_raw_payload}")
				else:
					print("No frm_payload available.")

				# Extract metadata like RSSI, SNR, and gateway info
				if 'rx_metadata' in uplink_message:
					for metadata in uplink_message['rx_metadata']:
						gateway_id = metadata['gateway_ids'].get('gateway_id', 'N/A')
						rssi = metadata.get('rssi', 'N/A')
						snr = metadata.get('snr', 'N/A')
						timestamp = metadata.get('timestamp', 'N/A')
						print(f"Gateway: {gateway_id}, RSSI: {rssi}, SNR: {snr}, Timestamp: {timestamp}")

				# Extract device info
				end_device_ids = parsed_record['data'].get('end_device_ids', {})
				device_id = end_device_ids.get('device_id', 'N/A')
				dev_eui = end_device_ids.get('dev_eui', 'N/A')
				print(f"Device ID: {device_id}, DevEUI: {dev_eui}")

				# Extract timestamp
				received_at = parsed_record['data'].get('received_at', 'N/A')
				print(f"Received At: {received_at}")

			else:
				print("Warning: 'data' key is missing in this record.")

		except json.JSONDecodeError as e:
			print(f"Failed to parse JSON object: {e.msg}")
		except KeyError as key_error:
			print(f"KeyError: {key_error} in record: {parsed_record}")

	return parsed_data
