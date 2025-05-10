#include <xml/parser>
#include <xml/serializer>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include "improved_note.h"

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
        cout << "  Priority: " << note.priority << endl;
        cout << "  Sent Date: " << note.sent_date.toString() << endl;
        
        // Modify priority field
        cout << "\nEnter new priority (current: " << note.priority << "): ";
        string newPriority;
        getline(cin, newPriority);
        
        if (!newPriority.empty()) {
            try {
                note.priority = stod(newPriority);
            } catch (const exception& e) {
                cerr << "Invalid number format. Priority not changed." << endl;
            }
        }
        
        // Modify date field
        cout << "Enter new date in YYYY-MM-DD format (current: " << note.sent_date.toString() << "): ";
        string newDate;
        getline(cin, newDate);
        
        if (!newDate.empty()) {
            try {
                note.sent_date = Date::fromString(newDate);
            } catch (const exception& e) {
                cerr << "Invalid date format. Date not changed." << endl;
            }
        }
        
        // Modify heading
        cout << "\nEnter new heading (or press Enter to keep current): ";
        string newHeading;
        getline(cin, newHeading);
        
        if (!newHeading.empty()) {
            note.heading = newHeading;
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