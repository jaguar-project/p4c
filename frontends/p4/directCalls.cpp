#include "directCalls.h"

namespace P4 {

const IR::Node* DoInstantiateCalls::postorder(IR::P4Parser* parser) {
    // std::cout << "DoInstantiateCalls::postorder(IR::P4Parser* parser = " << parser << std::endl;
    std::cout << "=====================Control Block Info========================" << std::endl;
    insert.append(parser->parserLocals);
    parser->parserLocals = insert;
    insert.clear();
    return parser;
}

const IR::Node* DoInstantiateCalls::postorder(IR::P4Control* control) {
    std::cout << "control block name is " << control->getName() << std::endl;
    std::cout << "control block body is " << control->body << std::endl;
    std::cout << "control->body->components.size() = " << control->body->components.size() << std::endl;
    for (int i = 0; i < control->body->components.size(); i++) {
        std::cout << "control->body->components[i] = " << control->body->components[i] << std::endl;
        std::cout << "control->body->components[i]->getNode()->node_type_name() = " << control->body->components[i]->getNode()->node_type_name() << std::endl;
        if (auto switch_ptr = control->body->components[i]->to<IR::SwitchStatement>()) {
           std::cout << "==================Output more about the block info==================" << std::endl;
           std::cout << "switch_ptr->expression->expression = " << switch_ptr->expression << std::endl;
           for (int j = 0; j < switch_ptr->cases.size(); j++) {
               std::cout << "switch_ptr->cases[j]->label = " << switch_ptr->cases[j]->label << std::endl;
               std::cout << "switch_ptr->cases[j]->statement = " << switch_ptr->cases[j]->statement << std::endl;
           }
        } else {
        }
    }
    std::cout << "\n";
    insert.append(control->controlLocals);
    control->controlLocals = insert;
    insert.clear();
    return control;
}

const IR::Node* DoInstantiateCalls::postorder(IR::MethodCallExpression* expression) {
    // std::cout << "DoInstantiateCalls::postorder(IR::MethodCallExpression* expression = " << expression << std::endl;
    // Identify type.apply(...) methods
    auto mem = expression->method->to<IR::Member>();
    if (mem == nullptr)
        return expression;
    auto tn = mem->expr->to<IR::TypeNameExpression>();
    if (tn == nullptr)
        return expression;

    auto ref = refMap->getDeclaration(tn->typeName->path, true);
    if (!ref->is<IR::P4Control>() && !ref->is<IR::P4Parser>()) {
        ::error("%1%: cannot invoke method of %2%", expression, ref);
        return expression;
    }

    auto name = refMap->newName(tn->typeName->path->name + "_inst");
    LOG3("Inserting instance " << name);
    auto annos = new IR::Annotations();
    annos->add(new IR::Annotation(
        IR::Annotation::nameAnnotation,
        { new IR::StringLiteral(tn->typeName->path->toString()) }, false));
    auto inst = new IR::Declaration_Instance(
        expression->srcInfo, IR::ID(name), annos,
        tn->typeName->clone(), new IR::Vector<IR::Argument>());
    insert.push_back(inst);

    auto path = new IR::PathExpression(expression->srcInfo,
                                       new IR::Path(IR::ID(expression->srcInfo, name)));
    expression->method = new IR::Member(path, mem->member);
    return expression;
}

}  // namespace P4
