#ifndef _BACKENDS_BMV2_JSONCONVERTER_H_
#define _BACKENDS_BMV2_JSONCONVERTER_H_

#include "lib/json.h"
#include "frontends/common/options.h"
#include "frontends/p4/evaluator/blockMap.h"
#include "frontends/p4/fromv1.0/v1model.h"
#include "analyzer.h"
// Currently we are requiring a v1model to be used

// This is based on the specification of the BMv2 JSON input format
// https://github.com/p4lang/behavioral-model/blob/master/docs/JSON_format.md

namespace BMV2 {

class ExpressionConverter;

class DirectMeterMap final {
 public:
    struct DirectMeterInfo {
        const IR::Expression* destinationField;
        const IR::P4Table* table;
        unsigned tableSize;

        DirectMeterInfo() : destinationField(nullptr), table(nullptr), tableSize(0) {}
    };

 private:
    // key is declaration of direct meter
    std::map<const IR::IDeclaration*, DirectMeterInfo*> directMeter;
    DirectMeterInfo* createInfo(const IR::IDeclaration* meter);
 public:
    DirectMeterInfo* getInfo(const IR::IDeclaration* meter);
    void setDestination(const IR::IDeclaration* meter, const IR::Expression* destination);
    void setTable(const IR::IDeclaration* meter, const IR::P4Table* table);
    void setSize(const IR::IDeclaration* meter, unsigned size);
};

class JsonConverter final {
 public:
    const CompilerOptions& options;
    Util::JsonObject       toplevel;  // output is constructed here
    P4V1::V1Model&         v1model;
    P4::P4CoreLibrary&     corelib;
    P4::ReferenceMap*      refMap;
    P4::TypeMap*           typeMap;
    ProgramParts           structure;
    cstring                dropAction = ".drop";
    unsigned               dropActionId;
    P4::BlockMap*          blockMap;
    ExpressionConverter*   conv;
    DirectMeterMap         meterMap;
    const IR::Parameter*   headerParameter;
    const IR::Parameter*   userMetadataParameter;
    const IR::Parameter*   stdMetadataParameter;

 protected:
    Util::IJson* typeToJson(const IR::Type_StructLike* type);
    unsigned nextId(cstring group);
    void addHeaderStacks(const IR::Type_Struct* headersStruct,
                         Util::JsonArray* headers, Util::JsonArray* headerTypes,
                         Util::JsonArray* stacks, std::set<cstring> &headerTypesCreated);
    void addTypesAndInstances(const IR::Type_StructLike* type, bool meta,
                              Util::JsonArray* headerTypes, Util::JsonArray* instances,
                              std::set<cstring> &headerTypesCreated);
    void convertActionBody(const IR::Vector<IR::StatOrDecl>* body,
                           Util::JsonArray* result, Util::JsonArray* fieldLists,
                           Util::JsonArray* calculations, Util::JsonArray* learn_lists);
    Util::IJson* convertTable(const CFG::TableNode* node, Util::JsonArray* counters);
    Util::IJson* convertIf(const CFG::IfNode* node, cstring parent);
    Util::JsonArray* createActions(Util::JsonArray* fieldLists, Util::JsonArray* calculations,
                                   Util::JsonArray* learn_lists);
    Util::IJson* toJson(const IR::P4Parser* cont);
    Util::IJson* toJson(const IR::ParserState* state);
    void convertDeparserBody(const IR::Vector<IR::StatOrDecl>* body, Util::JsonArray* result);
    Util::IJson* convertDeparser(const IR::P4Control* state);
    Util::IJson* convertParserStatement(const IR::StatOrDecl* stat);
    Util::IJson* convertControl(const IR::ControlBlock* block, cstring name,
                                Util::JsonArray* counters, Util::JsonArray* meters,
                                Util::JsonArray* registers);
    cstring createCalculation(cstring algo, const IR::Expression* fields,
                              Util::JsonArray* calculations);
    Util::IJson* nodeName(const CFG::Node* node) const;
    void createForceArith(const IR::Type* stdMetaType, cstring name,
                          Util::JsonArray* force_list) const;
    cstring convertHashAlgorithm(cstring algorithm) const;
    void handleTableImplementation(const IR::TableProperty* implementation,
                                   const IR::Key* key,
                                   Util::JsonObject* table);
    void addToFieldList(const IR::Expression* expr, Util::JsonArray* fl);
    // returns id of created field list
    int createFieldList(const IR::Expression* expr, cstring group,
                        cstring listName, Util::JsonArray* fieldLists);
    void generateUpdate(const IR::P4Control* cont,
                        Util::JsonArray* checksums, Util::JsonArray* calculations);

    // Operates on a select keyset
    void convertSimpleKey(const IR::Expression* keySet,
                          mpz_class& value, mpz_class& mask) const;
    unsigned combine(const IR::Expression* keySet,
                     const IR::ListExpression* select,
                     mpz_class& value, mpz_class& mask) const;
    void buildCfg(IR::P4Control* cont);

 public:
    explicit JsonConverter(const CompilerOptions& options);
    void convert(P4::BlockMap *blockMap);
    void serialize(std::ostream& out) const
    { toplevel.serialize(out); }
};

}  // namespace BMV2

#endif /* _BACKENDS_BMV2_JSONCONVERTER_H_ */