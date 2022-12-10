#!/bin/bash
#Used crontab -e to set up automation for this

#Display names and dates of left directory before changes
ls -l --time-style="+%c" /home/username/project/data/left

#Go into left directory
cd project
cd data
cd left


#Changing the names of the files in left directory to 1-n in random, content is randomly assigned
n=1; shuf -z -e -- *| while IFS= read -rd '' k; do mv -n "$k" "$((n++))"; done

#Randomize the dates in the left directory to last 10 days
for d in {1..16..1}
do
  yyyy=$(shuf -i 2022-2022 -n 1)
  mm=$(shuf -i 11-12 -n 1)
  dd=$(shuf -i 20-30 -n 1)
  hr=$(shuf -i 1-9 -n 1)
  mn=$(shuf -i 10-59 -n 1)
  ss=$(shuf -i 10-59 -n 1)
  touch -t "$yyyy"0"$mm""$dd""$hr""$mn" /home/username/project/data/left/"$d"
done
#Display names and dates of left directory after changes
ls -l --time-style="+%c" /home/username/project/data/left

cd --

#Display names and dates of right directory before changes
ls -l --time-style="+%c" /home/username/project/data/right

#Go into right directory
cd project
cd data
cd right


#Changing the names of the files in right directory to 1-n in random, content is randomly assigned
n=1; shuf -z -e -- *| while IFS= read -rd '' k; do mv -n "$k" "$((n++))"; done

#Randomize the dates in the right directory to last 10 days
for d in {1..6..1}
do
  yyyy=$(shuf -i 2022-2022 -n 1)
  mm=$(shuf -i 11-12 -n 1)
  dd=$(shuf -i 10-30 -n 1)
  hr=$(shuf -i 1-9 -n 1)
  mn=$(shuf -i 10-59 -n 1)
  ss=$(shuf -i 10-59 -n 1)
  touch -t "$yyyy"0"$mm""$dd""$hr""$mn" /home/username/project/data/right/"$d"
done

cd --

#Display names and dates of right directory after changes
ls -l --time-style="+%c" /home/username/project/data/right
