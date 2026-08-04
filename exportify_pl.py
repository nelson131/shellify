import csv
import os

# exporting spotify playlist into .shellify form for importing
# using exportify.net for example

csv_path = input("csv file -> ")

playlist_name = os.path.splitext(os.path.basename(csv_path))[0]

with open(csv_path, newline="", encoding="utf-8") as csv_file:
    reader = csv.DictReader(csv_file)

    with open("import.shellify", "w", encoding="utf-8") as out:
        out.write(f"{playlist_name}:\n")

        for row in reader:
            title = row["Track Name"]
            artist = row["Artist Name(s)"]
            album = row["Album Name"]

            out.write(f"{title}|{artist}|{album}\n")
