# Generate rules.txt from rules.grm or gules.md

sed '/export/!d;s/^\s*export\s\+\(\w\+\)\s\+=.*$/\1/g' $1