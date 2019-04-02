#!/usr/local/cs/bin/python

import csv
import sys
import string 

exitStatus = 0
allocated_inodes=set([])
class superBlock:
    def __init__(self, line):
        self.num_blocks = int(line[1])
        self.num_inodes = int(line[2])
        self.num_free_blocks = int(line[5])
        self.num_free_inodes = int(line[6])
        self.first_non_reserved_inode = int(line[7])
        
class Inode:
    def __init__(self, line):
        self.num_inodes = int(line[1])
        self.file_type = line[2]
        if self.file_type  != 's':
            self.dirent = [int(line[i]) for i in range(12,24)] #list of dirent inodes
            self.indirect = [int(line[i]) for i in range(24,27)] #list of indirect inodes
        else:
            self.dirent = [int(line[12])] 
        self.group = int(line[5])
        self.link_count = int(line[6])

class Dirent:
    def __init__(self, line):
        self.p_inode_num = int(line[1])
        self.offset = line[2]
        self.inode_num = int(line[3])
        self.entry_len = line[4] 
        self.name_len = line[5]
        #self.rec_len = int(line[5]) ##might not need
        self.name = line[6]

class Indirect:
    def __init__(self, line):
        self.p_inode_num = int(line[1])
        self.level = int(line[2])
        self.block_offset = int(line[3])
        self.block_num = int(line[4])
        self.ref_num = int(line[5])

def blockAudits(super_block, group, block_free, inodes, indirects):
    blockHolder = {}
    for HOLDER in inodes:
        if HOLDER.file_type == 'f' or HOLDER.file_type == 'd':
            for i in range(12):
                offSet = 0
                b=HOLDER.dirent[i]
                if b==0:
                    offSet = offSet +1
                elif b<8:
                    print("RESERVED BLOCK {} IN INODE {} AT OFFSET {}".format(b, HOLDER.num_inodes, offSet))
                    exitStatus = 2
                    offSet = offSet +1
                elif b in blockHolder:
                    blockHolder[b].append("BLOCK")
                    blockHolder[b].append(HOLDER.num_inodes)
                    blockHolder[b].append(offSet)
                    offSet = offSet +1
                elif b >= super_block.num_blocks or b < 0:
                    print("INVALID BLOCK {} IN INODE {} AT OFFSET {}".format(b, HOLDER.num_inodes, offSet))
                    offSet = offSet +1
                    exitStatus = 2
                    
                else:
                    blockHolder[b] = []
                    blockHolder[b].append("BLOCK")
                    blockHolder[b].append(HOLDER.num_inodes)
                    blockHolder[b].append(offSet)
                    offSet = offSet +1
                

        BLOCK_TYPE = ""
        i = 0
        Names = {0: "INDIRECT BLOCK", 1: "DOUBLE INDIRECT BLOCK", 2: "TRIPLE INDIRECT BLOCK"}
        Offsets={0: 12, 1: 268, 2: 65804}
        while i < 3:
            if HOLDER.file_type == 'f' or HOLDER.file_type == 'd':
                BVALUE = HOLDER.indirect[i]
                if BVALUE != 0:
                    if BVALUE < 8:
                        print("RESERVED {} {} IN INODE {} AT OFFSET {}".format(Names[i],BVALUE, HOLDER.num_inodes,Offsets[i]))
                        exitStatus = 2
                    elif (BVALUE >= super_block.num_blocks or BVALUE < 0):
                        print("INVALID {} {} IN INODE {} AT OFFSET {}".format(Names[i],BVALUE, HOLDER.num_inodes,Offsets[i]))
                        exitStatus = 2
                    elif BVALUE in blockHolder:
                        blockHolder[BVALUE].append(Names[i])
                        blockHolder[BVALUE].append(HOLDER.num_inodes)
                        blockHolder[BVALUE].append(Offsets[i])
                    else:
                        blockHolder[BVALUE] = []
                        blockHolder[BVALUE].append(Names[i])
                        blockHolder[BVALUE].append(HOLDER.num_inodes)
                        blockHolder[BVALUE].append(Offsets[i])
            i += 1
    Names = {1: "BLOCK", 2: "INDIRECT BLOCK", 3: "DOUBLE INDIRECT BLOCK"}
    Offsets={1: 0, 2: 12, 3: 268}
    for it in indirects:
        e=it.level 
        BVALUE = it.ref_num
        if BVALUE != 0:
            if (BVALUE < 0 or BVALUE >= super_block.num_blocks):
                print("INVALID {} {} IN INODE {} AT OFFSET {}".format(Names[e],BVALUE, HOLDER.num_inodes, Offsets[e]))
                exitStatus = 2
            elif BVALUE < 8:
                print("RESERVED BLOCK {} {} IN INODE {} AT OFFSET {}".format(Names[e],BVALUE, HOLDER.num_inodes, Offsets[e]))
                exitStatus = 2
            elif BVALUE in blockHolder:
                blockHolder[BVALUE].append(Names[e])
                blockHolder[BVALUE].append(HOLDER.num_inodes)
                blockHolder[BVALUE].append(Offsets[e])
            else:
                blockHolder[BVALUE] = []
                blockHolder[BVALUE].append(Names[e])
                blockHolder[BVALUE].append(HOLDER.num_inodes)
                blockHolder[BVALUE].append(Offsets[e])
    i=8
    
    while i<super_block.num_blocks:
        if i not in block_free and i not in blockHolder:
            print("UNREFERENCED BLOCK {}".format(i))
            exitStatus = 2
        elif i in block_free and i in blockHolder:
            print("ALLOCATED BLOCK {} ON FREELIST".format(i))
            exitStatus = 2
        if i in blockHolder and len(blockHolder[i])>3:
            c=0
            while c<len(blockHolder[i]):
                print("DUPLICATE {} {} IN INODE {} AT OFFSET {}".format(blockHolder[i][c], i, blockHolder[i][c+1], blockHolder[i][c+2]))
                exitStatus = 2
                c+=3
        i+=1
    return





def inodeAudits(superBlock, inode_free, inodes):
    for i in inodes:
        if i.file_type != '0':
            if i.num_inodes in inode_free:
                print("ALLOCATED INODE {} ON FREELIST".format(i.num_inodes))
                exitStatus=2
            allocated_inodes.add(i.num_inodes)
        else:
            if i.num_inodes not in inode_free:
                print("UNALLOCATED INODE {} NOT ON FREELIST".format(i.num_inodes))
                exitStatus=2  
    for it in range(superBlock.first_non_reserved_inode, superBlock.num_inodes):
        if (it not in allocated_inodes) and (it not in inode_free):
            print("UNALLOCATED INODE {} NOT ON FREELIST".format(it))
            exitStatus=2

def direcAudits(superBlock, inodes, dirents):
    links=[]
    parents=[]
    for k in range(1,superBlock.num_inodes+1):
        links.append(0)
        parents.append(0)
    parents[2] = 2
    for d in dirents:
        if d.inode_num > superBlock.num_inodes:
            print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(d.p_inode_num,d.name,d.inode_num))
            exitStatus=2
        elif d.inode_num<1:
            print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(d.p_inode_num,d.name,d.inode_num))
            exitStatus=2
        elif d.inode_num not in allocated_inodes:
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(d.p_inode_num,d.name,d.inode_num))
            exitStatus=2
        else:
            links[d.inode_num]+=1
            if d.name!="'.'" and d.name!="'..'":
                parents[d.inode_num]=d.p_inode_num
    for d in dirents:       
        if d.name=="'.'":
                if d.p_inode_num != d.inode_num:
                    print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(d.p_inode_num, d.inode_num, d.p_inode_num))
                    exitStatus=2
        if d.name=="'..'":
            if d.inode_num != parents[d.p_inode_num]:
                print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(d.p_inode_num, d.inode_num, d.p_inode_num))
                exitStatus=2
    for i in inodes:
        if i.link_count != links[i.num_inodes]:
            print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(i.num_inodes, links[i.num_inodes], i.link_count))
            exitStatus=2
    return

def main():
    if len(sys.argv) != 2:
        sys.stderr.write("ERROR, wrong number of arguments Correct usage: ./lab3b CSV_FILENAME\n")
        sys.exit(1)
    
    block_free = []
    inode_free = []
    inodes = []
    dirents = []
    indirects = []
    sBlockS = None
    group = None
    
    try:
        with open(sys.argv[1], 'r') as csvfile:
            reader = csv.reader(csvfile)
            for line in reader:
                x = line[0]
                if x == 'BFREE':
                    block_free.append(int(line[1]))
                elif x == 'IFREE':
                    inode_free.append(int(line[1]))
                elif x == 'INODE':
                    inodes.append(Inode(line))
                elif x == 'SUPERBLOCK':
                    sBlockS = superBlock(line)
                elif x == 'DIRENT':
                    dirents.append(Dirent(line))
                elif x == 'INDIRECT':
                    indirects.append(Indirect(line))
    except (IOError, OSError):
        sys.stderr.write("ERROR in file reading\n")
        sys.exit(1)

    blockAudits(sBlockS, group, block_free, inodes, indirects)
    inodeAudits(sBlockS, inode_free, inodes)
    direcAudits(sBlockS, inodes, dirents)
    sys.exit(exitStatus)

if __name__ == "__main__":
    main()
