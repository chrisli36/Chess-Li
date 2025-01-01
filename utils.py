def ctzll(x):
    return (x & -x).bit_length() - 1 if x != 0 else 64