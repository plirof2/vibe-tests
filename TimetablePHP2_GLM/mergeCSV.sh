#!/bin/bash

# Output file
output="merged.csv"

# Empty or create the output file
> "$output"

# Loop through all CSV files in the current directory
for file in *.csv; do
    # Skip the output file if it exists in the directory
    if [[ "$file" == "$output" ]]; then
        continue
    fi
    # Append filename in quotes
    echo "\"$file\"" >> "$output"
    # Append contents of the file
    cat "$file" >> "$output"
    # Add a newline for separation
    echo -e "\n" >> "$output"
done

echo "Merge complete. Output saved to $output"