/*
 * @Author: Choubin
 * @Date: 2020-06-25 14:28:12
 * @LastEditors: Choubin
 * @LastEditTime: 2020-06-25 17:58:33
 * @FilePath: /geduo/geduo/test.cpp
 * @Description:  
 */

#include <algorithm>
#include <iostream>
#include <json/json.h>
#include <stack>
#include <string>

using namespace std;

int main() {
    string test = "{\"id\":1,\"name\":\"test\"}";
    Json::Reader reader;
    Json::Value value;

    if (reader.parse(test, value)) {
        if (!value["id"].isNull()) {
            cout << value["id"].asInt() << endl;
            cout << value["name"].asString() << endl;
        }
    }
    return 0;
}
