#Section 2 Python Combine Program

def combine(a, b):
    return a + b

print(combine(5, 3))        # 8
print(combine("5", "3"))    # "53"
print(combine(5, "3"))      # TypeError: unsupported operand type(s)