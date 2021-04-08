#!/usr/bin/python3
import subprocess
import json
import sys

device = sys.argv[1]

r = subprocess.run(["smartctl", "-j", "-x", device], capture_output=True)
obj = json.loads(r.stdout)
print("device", device, sep=",")
print("metadata", obj["model_name"], obj["user_capacity"]["bytes"], 
        obj["rotation_rate"],
        obj["temperature"]["current"], sep=",")
for attr in obj["ata_smart_attributes"]["table"]:
    print(attr["name"], attr["value"], sep=",")
for event in obj["sata_phy_event_counters"]["table"]:
    print(event["name"], event["value"], sep=",")
