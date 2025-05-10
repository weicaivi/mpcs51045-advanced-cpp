#include <xml/parser>
#include <xml/serializer>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include "note.h"

using namespace std;
using namespace xml;

int main(int argc, char **argv)
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input-xml-file> <output-xml-file>" << endl;
        return 1;
    }

    try {
        // Read the note from the input file
        ifstream ifs(argv[1]);
        if (!ifs) {
            cerr << "Error: Cannot open input file: " << argv[1] << endl;
            return 1;
        }
        
        parser p(ifs, "xml");
        note_type note = fromXML<note_type>(p);
        
        // Display the current values
        cout << "Original note:" << endl;
        cout << "  From: " << note.from << endl;
        cout << "  To: " << note.to << endl;
        if (note.cc) {
            cout << "  CC: " << note.cc.value() << endl;
        }
        cout << "  Heading: " << note.heading << endl;
        cout << "  Body: " << note.body.substr(0, 50) << "..." << endl;
        
        // Modify a field (for example, change the heading)
        cout << "\nEnter new heading (or press Enter to keep current): ";
        string newHeading;
        getline(cin, newHeading);
        
        if (!newHeading.empty()) {
            note.heading = newHeading;
        }
        
        if (note.cc) {
            cout << "Current CC: " << note.cc.value() << endl;
            cout << "Enter new CC (or press Enter to keep current, or type 'none' to remove): ";
        } else {
            cout << "Enter CC (or press Enter for none): ";
        }
        
        string newCC;
        getline(cin, newCC);
        
        if (!newCC.empty()) {
            if (newCC == "none") {
                note.cc.reset(); // Remove the CC field
            } else {
                note.cc = newCC; // Set or update the CC field
            }
        }
        
        // Write the modified note to the output file
        ofstream ofs(argv[2]);
        if (!ofs) {
            cerr << "Error: Cannot open output file: " << argv[2] << endl;
            return 1;
        }
        
        serializer s(ofs, "xml");
        toXML(note, s);
        
        cout << "\nNote has been modified and saved to " << argv[2] << endl;
        
    } catch (std::exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}