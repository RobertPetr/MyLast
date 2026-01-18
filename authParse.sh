#!/bin/bash

filepath="/var/log/auth.log"
b="$1"

while IFS= read -r linie; do
    IFS=' ' read -r data host command message <<< "$linie"

    data=$(date -d "$data" +"%s")

    user=""
    line=""
    flag=0

    if [[ "$command" == *"gdm-password"* && "$b" == "/var/log/wtmp" ]]; then
        if [[ "$message" == *"session opened"* ]]; then
            user=$(echo "$message" | sed -n 's/.*user \([^ ]*\)(uid=[0-9]*).*/\1/p')
            line="tty2"
            flag=1
        fi
    elif [[ "$command" == *"systemd-logind"* && "$message" == *"System is powering down."* && "$b" == "/var/log/wtmp" ]]; then
        user="reboot"
        line="~"
        flag=1
    elif [[ "$command" == *"unix_chkpwd"* && "$b" == "/var/log/btmp" ]]; then
        user=$(echo "$message" | awk '{print $6}' | sed 's/[()]//g')
        line="seat0"
        flag=1
    fi
    printf "%d,%s,%s,%s,%ld,0\n" "$flag" "$user" "$line" "$host" "$data"
done < <(tac "$filepath")