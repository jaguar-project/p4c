/*
Copyright 2016 VMware, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "core.p4"

extern E {
    bit<32> call();
}

control c() {
    action a() {}
    table t(in E e) {
        key = { e.call() : exact; }
        actions = { a; }
        default_action = a;
    }
    E() einst;
    apply {
        t.apply(einst);
    }
}

control none();
package top(none n);

top(c()) main;
