#include <xml/parser>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "note.h"

using namespace std;
using namespace xml;
int main(int argc, char **argv)
{
    ifstream ifs(argv[1]);
    parser p(ifs, "xml");
    try {
        note_type note = fromXML<note_type>(p);
        cout << "Message from " << note.from << " to " << note.to << endl;
    } catch (std::exception &e) {
        cout << e.what();
    }
}

