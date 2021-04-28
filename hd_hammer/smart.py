#!/usr/bin/python3
import sys
import subprocess
import json
import os

# If given no filename, print out difference for all smart data 
if len(sys.argv) == 1:
    for file_name in os.listdir("smart"):
        if file_name == ".gitignore":
            continue
        print(file_name, end="")
        file_name = "smart/"+file_name

        before = {}
        after = {}
        with open(file_name) as f:
            current = before
            for line in f.readlines():
                line = line.strip()
                if line.startswith("before") or line.startswith("device") or line.startswith("metadata") or line.startswith("name") or line.startswith("#"):
                    continue
                elif line.startswith("after"):
                    current = after
                else:
                    parts = line.rsplit(",", 1)
                    current[parts[0]] = parts[1]

        end = False
        for key in before:
            if key in after and before[key] != after[key]:
                if not end:
                    print()
                end = True
                print("\t", key, before[key], after[key], sep="\t")
        if not end:
            print()
else: #otherwise create smart data for filename

    device = sys.argv[1]

    r = subprocess.run(["smartctl", "-j", "-x", device], capture_output=True)
    obj = json.loads(r.stdout)
    print("#device", device, sep=",")
    print("#metadata", obj["model_name"], obj["user_capacity"]["bytes"], 
            obj["rotation_rate"],
            obj["temperature"]["current"], sep=",")
    print("name,value,id")
    for attr in obj["ata_smart_attributes"]["table"]:
        print(attr["name"], attr["value"], attr["id"], sep=",")
    for event in obj["sata_phy_event_counters"]["table"]:
        print(event["name"], event["value"], event["id"], sep=",")


