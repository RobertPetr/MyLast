#!/bin/bash

filepath="/var/log/auth.log"
b="$1"

cat tac "$filepath" | while IFS= read -r linie; do
    if [[ "b" == "/var/log/btmp" ]]; then
        if grep -q "Failed password" "$linie"; then
            read -r luna zi ora host proces mesaj <<< "$linie"
            read -r failed password for user _ <<< "$mesaj"
            data=$(date -d "$luna $zi $ora" +"%s")
            printf "1,%s,%s,%s,%ld,%ld" "$user" "SSH" "$host" "$data" "0"
        fi
        if grep -q "Invalid user" "$linie"; then
            read -r luna zi ora host proces mesaj <<< "$linie"
            read -r invalid u user _ <<< "$mesaj"
            data=$(date -d "$luna $zi $ora" +"%s")
            printf "1,%s,%s,%s,%ld,%ld" "$user" "SSH" "$host" "$data" "0"
        fi
    else
        if grep -q "Accepted" "$linie"; then
            read -r luna zi ora host proces mesaj <<< "$linie"
            read -r Accepted password for user _ <<< "$mesaj"
            data=$(date -d "$luna $zi $ora" +"%s")
            printf "1,%s,%s,%s,%ld,%ld" "$user" "SSH" "$host" "$data" "0"
        fi
        if grep -q "login" "$linie"; then
            read -r luna zi ora host proces mesaj <<< "$linie"
            read -r pam_unix session opened for u user _ <<< "$mesaj"
            data=$(date -d "$luna $zi $ora" +"%s")
            printf "1,%s,%s,%s,%ld,%ld" "$user" "SSH" "$host" "$data" "0"
        fi
    fi
done