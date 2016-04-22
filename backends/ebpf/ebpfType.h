#ifndef _BACKENDS_EBPF_EBPFTYPE_H_
#define _BACKENDS_EBPF_EBPFTYPE_H_

#include "lib/algorithm.h"
#include "lib/sourceCodeBuilder.h"
#include "ebpfObject.h"
#include "ir/ir.h"

namespace EBPF {

// Base class for EBPF types
class EBPFType : public EBPFObject {
 protected:
    explicit EBPFType(const IR::Type* type) : type(type) {}
 public:
    const IR::Type* type;
    virtual void emit(CodeBuilder* builder) = 0;
    virtual void declare(CodeBuilder* builder,
                         cstring id, bool asPointer) = 0;
    virtual void emitInitializer(CodeBuilder* builder) = 0;
    virtual void declareArray(CodeBuilder* ,
                              const char* /*id*/, unsigned /*size*/)
    { BUG("Arrays of %1% not supported", type); }
    cstring toString(const Target* target);
};

class IHasWidth {
 public:
    virtual ~IHasWidth() {}
    // P4 width
    virtual unsigned widthInBits() = 0;
    // Width in the target implementation.
    // Currently a multiple of 8.
    virtual unsigned implementationWidthInBits() = 0;
};

class EBPFTypeFactory {
 private:
    const P4::TypeMap* typeMap;
    explicit EBPFTypeFactory(const P4::TypeMap* typeMap) : typeMap(typeMap) {}
 public:
    static EBPFTypeFactory* instance;
    static void createFactory(P4::TypeMap* typeMap)
    { EBPFTypeFactory::instance = new EBPFTypeFactory(typeMap); }
    EBPFType* create(const IR::Type* type);
};

class EBPFBoolType : public EBPFType, IHasWidth {
 public:
    EBPFBoolType() : EBPFType(IR::Type_Boolean::get()) {}
    void emit(CodeBuilder* builder) override
    { builder->append("u8"); }
    void declare(CodeBuilder* builder,
                 cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override
    { builder->append("0"); }
    unsigned widthInBits() override { return 1; }
    unsigned implementationWidthInBits() override { return 8; }
};

class EBPFScalarType : public EBPFType, public IHasWidth {
 public:
    const unsigned width;
    const bool     isSigned;
    explicit EBPFScalarType(const IR::Type_Bits* bits) :
            EBPFType(bits), width(bits->size), isSigned(bits->isSigned) {
    }
    unsigned bytesRequired() const { return ROUNDUP(width, 8); }
    unsigned alignment() const;
    void emit(CodeBuilder* builder) override;
    void declare(CodeBuilder* builder,
                 cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override
    { builder->append("0"); }
    unsigned widthInBits() override { return width; }
    unsigned implementationWidthInBits() override { return bytesRequired() * 8; }
    // True if this width is small enough to store in a machine scalar
    static bool generatesScalar(unsigned width)
    { return width <= 32; }
};

// This should not always implement IHasWidth, but it may...
class EBPFTypeName : public EBPFType, public IHasWidth {
    const IR::Type_Name* type;
    EBPFType* canonical;
 public:
    EBPFTypeName(const IR::Type_Name* type, EBPFType* canonical) :
            EBPFType(type), type(type), canonical(canonical) {}
    void emit(CodeBuilder* builder) { canonical->emit(builder); }
    void declare(CodeBuilder* builder, cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override;
    unsigned widthInBits() override;
    unsigned implementationWidthInBits() override;
};

// Also represents headers and unions
class EBPFStructType : public EBPFType, public IHasWidth {
    class EBPFField {
     public:
        EBPFType* type;
        const IR::StructField* field;

        EBPFField(EBPFType* type, const IR::StructField* field) :
                type(type), field(field) {}
    };

 public:
    cstring  kind;
    cstring  name;
    std::vector<EBPFField*>  fields;
    unsigned width;
    unsigned implWidth;

    explicit EBPFStructType(const IR::Type_StructLike* strct);
    void declare(CodeBuilder* builder, cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override;
    unsigned widthInBits() override { return width; }
    unsigned implementationWidthInBits() override { return implWidth; }
    void emit(CodeBuilder* builder) override;
};

}  // namespace EBPF

#endif /* _BACKENDS_EBPF_EBPFTYPE_H_ */