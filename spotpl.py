from spotipy import Spotify
from spotipy.oauth2 import SpotifyClientCredentials

def get_client_info() -> dict[str, str]:
    res = {}
    with open("spot.env", "r") as file:
        lines =  file.readlines()
        if len(lines) < 3:
            raise ValueError("spot.env less than 3 lines")
        res["CLIENT_ID"] = lines[0].strip()
        res["CLIENT_SECRET"] = lines[1].strip()
        res["PLAYLIST_URL"] = lines[2].strip()

    return res


def main():
    client_info = get_client_info()
    CLIENT_ID = client_info["CLIENT_ID"]
    CLIENT_SECRET = client_info["CLIENT_SECRET"]
    PLAYLIST_URL = client_info["PLAYLIST_URL"]

    spotify = Spotify(
        auth_manager=SpotifyClientCredentials(
            client_id=CLIENT_ID,
            client_secret=CLIENT_SECRET
        )
    )

    playlist = spotify.playlist(PLAYLIST_URL)
    playlist_name = playlist["name"]

    with open("imported_plist.shellify", "w", encoding="utf-8") as file:
        file.write(f"{playlist_name}:\n")

        while True:
            for i in playlist["tracks"]["items"]:
                track = i["track"]
                if track is None:
                    continue

                title = track["name"]
                artist = ", ".join(a["name"] for a in track["artists"])
                album = track["album"]["name"]

                file.write(f"{title}|{artist}|{album}\n")
            
            if playlist["tracks"]["next"]:
                playlist["tracks"] = spotify.next(playlist["tracks"])
            else:
                break

if __name__ == "__main__":
    main()
