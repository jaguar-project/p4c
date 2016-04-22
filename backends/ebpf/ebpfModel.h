#ifndef _BACKENDS_EBPF_EBPFMODEL_H_
#define _BACKENDS_EBPF_EBPFMODEL_H_

#include "frontends/common/model.h"
#include "frontends/p4/coreLibrary.h"
#include "ir/ir.h"
#include "lib/cstring.h"

namespace EBPF {

struct TableImpl_Model : public ::Model::Extern_Model {
    explicit TableImpl_Model(cstring name) :
            Extern_Model(name),
            size("size") {}
    ::Model::Elem size;
};

struct CounterArray_Model : public ::Model::Extern_Model {
    CounterArray_Model() : Extern_Model("CounterArray"),
                           increment("increment"),
                           size("max_index"), sparse("sparse")  {}
    ::Model::Elem increment;
    ::Model::Elem size;
    ::Model::Elem sparse;
};

struct Filter_Model : public ::Model::Elem {
    Filter_Model() : Elem("ebpf_filter"),
                     parser("prs"), filter("filt") {}
    ::Model::Elem parser;
    ::Model::Elem filter;
};

// Keep this in sync with ebpf_model.p4
class EBPFModel : public ::Model::Model {
 protected:
    EBPFModel() : Model("0.1"),
                  counterArray(),
                  array_table("array_table"),
                  hash_table("hash_table"),
                  tableImplProperty("implementation"),
                  CPacketName("skb"),
                  packet("packet", P4::P4CoreLibrary::instance.packetIn, 0),
                  filter()
    {}

 public:
    static EBPFModel instance;
    static cstring reservedPrefix;

    CounterArray_Model     counterArray;
    TableImpl_Model        array_table;
    TableImpl_Model        hash_table;
    ::Model::Elem          tableImplProperty;
    ::Model::Elem          CPacketName;
    ::Model::Param_Model   packet;
    Filter_Model           filter;

    static const IR::Type* counterIndexType;
    static const IR::Type* counterValueType;

    static cstring reserved(cstring name)
    { return reservedPrefix + name; }
};

}  // namespace EBPF

#endif /* _BACKENDS_EBPF_EBPFMODEL_H_ */