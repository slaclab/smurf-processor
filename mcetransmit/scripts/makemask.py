#creates a generic  mask
def makemask():
    filename = "../mask.txt"
    num_pts = 528; 
    f = open(filename, 'w');
    print f
    for n in range(0, num_pts):
        f.write(str(n)+ "\n");
    f.close()

