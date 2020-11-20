import re

TableDict = {}
HeaderDict = {}
StructDict = {}
ActionDict = {}

f =  open("/tmp/example.txt", "r")
# key is table name and value is a list of list including 
# action list; match portion

#Header name = hop_metadata_t
#Header members:vrf;ipv6_prefix;next_hop_index;mcast_grp;urpf_fail;drop_reason; 

#Struct name = headers
#Struct members:

# Successor dependency:Table A’s match result determines whether Table B should be executed or not

# whether there is common member in both list1 and list2
def overlap(list1, list2):
    for x in list1:
        if x in list2:
            return True
    return False

# output one of the previous four dependencies
def output_relationship(tableA, tableB, TableDict, ActionDict):
    # Match dependencies
    modify_fields_tableA = []
    modify_fields_tableB = []
    for i in range(len(TableDict[tableA][1])):
        action_name = TableDict[tableA][1][i]
        for j in range(len(ActionDict[action_name])):
            modify_fields_tableA.append(ActionDict[action_name][j])

    for i in range(len(TableDict[tableB][1])):
        action_name = TableDict[tableB][1][i]
        for j in range(len(ActionDict[action_name])):
            modify_fields_tableB.append(ActionDict[action_name][j])
    match_fields_tableA = TableDict[tableA][0]
    match_fields_tableB = TableDict[tableB][0]
    # Match dependency: Table A modifies a field Table B matches
    if overlap(modify_fields_tableA, match_fields_tableB):
        print(tableA, "has Match dependency relationship with", tableB)
    # Action dependency: Table A and B both change the same field, but the end-result should be that of the later Table B
    elif overlap(modify_fields_tableA, modify_fields_tableB):
        print(tableA, "has Action dependency relationship with", tableB)
    # Reverse dependency: Table A matches on a field that Table B modifies,
    elif overlap(match_fields_tableA, modify_fields_tableB):
        print(tableA, "has Reverse dependency relationship with", tableB)
    else:
        print(tableA, "has no dependency relationship with", tableB)

line_list = []

for x in f:
  line_list.append(x)

for i in range(len(line_list)):
  x = line_list[i]
  if x == "---------\n":
     continue
  elif x.find('Header name =') != -1:
     if x[len(x) - 1] == '\n':
         x = x[:len(x) - 2]
     header_name = x.split(' = ')[1]
     #print("header_name = ", header_name)
     y = line_list[i + 1]
     if y[len(y) - 1] == '\n':
         y = y[:len(y) - 1]
     if y[len(y) - 1] != ":":
         header_member = y[:len(y)-1].split(":")[1].split(";")
         #print("header_member", header_member)
     else:
         header_member = []
         #print("header_member is empty")
     HeaderDict[header_name] = header_member
     i = i + 1
  elif x.find("Struct name =") != -1:
     if x[len(x) - 1] == '\n':
         x = x[:len(x) - 2]
     struct_name = x.split(' = ')[1]
     #print("struct_name = ", struct_name)
     y = line_list[i + 1]
     if y[len(y) - 1] == '\n':
         y = y[:len(y) - 1]
     if y[len(y) - 1] != ":":
         struct_member = y[:len(y)-1].split(":")[1].split(";")
         #print("struct_member", struct_member)
     else:
         struct_member = []
         #print("struct_member is empty")
     StructDict[struct_name] = struct_member
     i = i + 1
  elif x.find("Table name =") != -1:
     # Table name = smac_vlan
     # Match portion:hdr.ethernet.srcAddr;standard_metadata.ingress_port;
     # Action portion:
     table_name = x.split(' = ')[1]
     if table_name[len(table_name) - 1] == '\n':
         table_name = table_name[:len(table_name) - 1]
     #print("table_name = ", table_name)
     match_portion = line_list[i + 1]
     action_portion = line_list[i + 2]
     i = i + 2
     if match_portion[len(match_portion) - 1] == '\n':
         match_portion = match_portion[:len(match_portion) - 1]
     if match_portion[len(match_portion) - 1] != ":":
         match_member = match_portion[:len(match_portion) - 1].split(":")[1].split(";")
         #print("match_member = ", match_member)
     else:
         match_member = []
         #print("match_member is empty")

     if action_portion[len(action_portion) - 1] == '\n':
         action_portion = action_portion[:len(action_portion) - 1]
     if action_portion[len(action_portion) - 1] != ":":
         action_member = action_portion[:len(action_portion) - 1].split(":")[1].split(";")
         #print("action_member = ", action_member)
     else:
         action_member = []
         #print("action_member is empty")

     TableDict[table_name] = [match_member, action_member]
  elif x.find("Action name =") != -1: 
     # Action name = set_egress_port
     # fields modified within an Action:standard_metadata.egress_spec;
     action_name = x.split(" = ")[1]
     if action_name[len(action_name) - 1] == '\n':
         action_name = action_name[:len(action_name) - 1]
     #print("action_name = ", action_name)
     fields_portion = line_list[i + 1]
     if fields_portion[len(fields_portion) - 1] == '\n':
         fields_portion = fields_portion[:len(fields_portion) - 1]
     if fields_portion[len(fields_portion) - 1] != ":":
         fields_list = fields_portion[:len(fields_portion) - 1].split(":")[1].split(";")
         #print("fields_list = ", fields_list)
     else:
         fields_list = []
         #print("field_list is empty")
     i = i + 1
     ActionDict[action_name] = fields_list

print("TableDict", TableDict)
print("HeaderDict", HeaderDict)
print("StructDict", StructDict)
print("ActionDict", ActionDict)


f = open("/tmp/table_dep.txt", "r")
for x in f:
    if x.find("implemented before") != -1:
        # print("table_name",table_name.group(1,2,3,4))  output table_name ('ingress', 'smac_vlan', 'ingress', 'dmac_vlan')
        table_name = re.match("Table (\w+).(\w+) is implemented before Table (\w+).(\w+)\n",x)
        tableA = table_name.group(2)
        tableB = table_name.group(4)
        print("tableA = ", tableA)
        print("tableB = ", tableB)
        # TODO: figure out the dependency between tableA and tableB
        output_relationship(tableA, tableB, TableDict, ActionDict)
    else:
        assert x.find("Match result") != -1
        table_name = re.search("Match result of Table (\w+).(\w+) will decide whether to implement Table (\w+).(\w+) or not\n", x)
        # print("2 table_name", table_name.group(1,2,3,4))
        tableA = table_name.group(2)
        tableB = table_name.group(4)
        print(tableA, "has successor dependency relationship with", tableB)