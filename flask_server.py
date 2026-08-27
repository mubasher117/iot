from flask import Flask, jsonify, request
import requests

app = Flask(__name__)

# Device Configuration
DEVICE_CONFIG = {
    "small_room": {
        "ip": "YOUR_IP",  # Assign your Gate ESP32 IP here
        "type": "room"
    },
    "gate": {
        "ip": "YOUR_IP",  # Assign your Gate ESP32 IP here
        "type": "trigger"
    }
}

# ==========================================
# 1. GET FULL HOME STATE (Queries Rooms Only)
# ==========================================
@app.route('/state', methods=['GET'])
def get_home_state():
    full_home_state = {}

    for device_name, config in DEVICE_CONFIG.items():
        # Skip trigger-only devices like the gate during full state polling
        if config.get("type") == "trigger":
            continue

        try:
            # Query physical status from ESP32 /status endpoint
            response = requests.get(f"http://{config['ip']}/status", timeout=2)
            if response.status_code == 200:
                full_home_state[device_name] = response.json()
            else:
                full_home_state[device_name] = {"error": "Invalid response"}
        except requests.exceptions.RequestException:
            full_home_state[device_name] = {"error": "Room ESP32 Offline"}

    return jsonify(full_home_state), 200


# ==========================================
# 2. DEDICATED GATE TRIGGER ENDPOINT
# ==========================================
@app.route('/gate/open', methods=['POST', 'GET'])
def trigger_gate():
    gate_ip = DEVICE_CONFIG["gate"]["ip"]
    try:
        # Pulse the gate relay via ESP32
        response = requests.get(f"http://{gate_ip}/trigger-gate", timeout=3)
        if response.status_code == 200:
            return jsonify({"success": True, "message": "Gate open signal sent"}), 200
        else:
            return jsonify({"error": "Gate ESP32 rejected command"}), 502
    except requests.exceptions.RequestException:
        return jsonify({"error": f"Could not reach Gate ESP32 at {gate_ip}"}), 504


# ==========================================
# 3. POST COMMAND TO ROOM
# ==========================================
@app.route('/<room>/<appliance>/<state>', methods=['POST', 'GET'])
def control_appliance(room, appliance, state):
    room = room.lower()
    appliance = appliance.lower()
    state = state.lower()

    if room not in DEVICE_CONFIG or DEVICE_CONFIG[room].get("type") == "trigger":
        return jsonify({"error": f"Room '{room}' not found"}), 404

    if state not in ["on", "off"]:
        return jsonify({"error": "State must be 'on' or 'off'"}), 400

    target_state = 1 if state == "on" else 0
    esp_ip = DEVICE_CONFIG[room]["ip"]

    try:
        # Send command to ESP32
        esp_url = f"http://{esp_ip}/set?appliance={appliance}&state={target_state}"
        response = requests.get(esp_url, timeout=3)

        if response.status_code == 200:
            # Re-fetch full home state directly from hardware pins
            return get_home_state()
        else:
            return jsonify({"error": "ESP32 hardware command failed"}), 502

    except requests.exceptions.RequestException:
        return jsonify({"error": f"Could not reach {room} at {esp_ip}"}), 504


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)