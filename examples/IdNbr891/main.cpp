#include <iostream>
#include "tinyxml2.h"

using namespace std;

void create_XML();

int main()
{
    create_XML();
    return 0;
}

void create_XML()
{
    tinyxml2::XMLDocument *xmlDoc = new tinyxml2::XMLDocument;
    auto declaration = xmlDoc->NewDeclaration();
    tinyxml2::XMLNode *root = xmlDoc->NewElement("root");
    xmlDoc->InsertFirstChild(declaration);
    xmlDoc->InsertAfterChild(declaration, root);

    tinyxml2::XMLElement *save_date = xmlDoc->NewElement("save_date");
    root->InsertEndChild(save_date);
    save_date->SetText("2022-01-01");
    xmlDoc->SaveFile("/tmp/myxml.xml");
    delete xmlDoc;
}
