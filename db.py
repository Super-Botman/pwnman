file = open("./test.db", "wb")
header = b"B0T\x00"

file.write(header)
