def compare_binary_files(file1, file2):
    with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
        data1 = f1.read()
        data2 = f2.read()

    if len(data1) != len(data2):
        raise ValueError("Files have different lengths")

    modified_ranges = []
    start_address = None
    current_address = None

    for i in range(0, len(data1), 3):
        pack1 = data1[i:i+3]
        pack2 = data2[i:i+3]

        if pack1 != pack2:
            if start_address is None:
                start_address = i // 3
            current_address = i // 3
        elif start_address is not None:
            end_address = current_address
            modified_ranges.append((start_address, end_address))
            start_address = None

    if start_address is not None:
        modified_ranges.append((start_address, current_address))

    return modified_ranges

def print_modified_ranges(file1, file2):
    modified_ranges = compare_binary_files(file1, file2)

    if not modified_ranges:
        print("No differences found.")
    else:
        print("Modified ranges:")
        for start, end in modified_ranges:
            print(f"Address Range: 0x{start:04X} - 0x{end:04X}")

print_modified_ranges("PDC32.firmware", "PDC32.firmware.old")