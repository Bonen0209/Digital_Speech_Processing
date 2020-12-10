import sys

def main(argv, arc):
    input_filename = argv[1]
    output_filename = argv[2]

    d_outputs = {}
    with open(input_filename, 'r', encoding='big5hkscs') as f:
        for line in f:
            temp_line = line.split()
            char = temp_line[0]
            phon_alphas = temp_line[1].split('\\')
            for phon_alpha in phon_alphas:
                if phon_alpha[0] not in d_outputs:
                    d_outputs[phon_alpha[0]] = set()
                d_outputs[phon_alpha[0]].add(char)

    with open(output_filename, 'w', encoding='big5hkscs') as f:
        for phon_alpha, s_char in d_outputs.items():
            chars = list(s_char)
            f.write(f'{phon_alpha} {" ".join(chars)}\n')
            for char in chars:
                f.write(f'{char} {char}\n')

if __name__ == '__main__':
    main(sys.argv, len(sys.argv))
