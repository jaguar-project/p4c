#include "ir.h"

void IR::Node::traceVisit(const char* visitor) const
{ LOG3("Visiting " << visitor << " " << id << ":" << node_type_name()); }

void IR::Node::traceCreation() const { LOG5("Created node " << id); }

int IR::Node::currentId = 0;

IRNODE_DEFINE_APPLY_OVERLOAD(Node, , )