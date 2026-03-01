//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Ebony Anderson
// Date        : 02.19.2026
// Description : 7-1 Submit Project Two
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <limits>
#include <cctype>

using namespace std;

// -------------------- Data Model --------------------
// Represents a single course with its number, title, and prerequisite IDs.
struct Course {
    string courseNumber;          // e.g., "CSCI100"
    string name;                  // e.g., "Introduction to Computer Science"
    vector<string> prerequisites; // list of prerequisite course numbers
};

// -------------------- Helper Functions --------------------

// Trim leading and trailing whitespace from a string.
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Convert a string to uppercase (for case-insensitive course input).
string toUpper(const string& input) {
    string result = input;
    transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(toupper(c)); });
    return result;
}

// -------------------- File Loading --------------------
// Load courses from a CSV file into the provided hash table.
//
// Two-pass strategy:
//  1. First pass: collect all courseNumbers so we can validate prerequisite IDs.
//  2. Second pass: build Course objects, validate prerequisites, and insert them.
//
// Returns true on success, false on error.
bool loadCourses(unordered_map<string, Course>& courseTable, const string& fileName) {
    ifstream infile(fileName);
    if (!infile.is_open()) {
        cout << "Error: Could not open file '" << fileName << "' for reading." << endl;
        return false;
    }

    unordered_set<string> courseIds;
    string line;

    // ---------- FIRST PASS: collect all course IDs ----------
    while (getline(infile, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, ',')) {
            tokens.push_back(trim(token));
        }

        if (tokens.size() < 2) {
            cout << "Format warning: line must contain at least a course number and course name." << endl;
            continue;
        }

        const string& courseNumber = tokens[0];
        if (!courseNumber.empty()) {
            courseIds.insert(courseNumber);
        }
    }

    infile.close();

    // ---------- SECOND PASS: parse courses and validate prerequisites ----------
    ifstream infile2(fileName);
    if (!infile2.is_open()) {
        cout << "Error: Could not reopen file '" << fileName << "' for second pass." << endl;
        return false;
    }

    // Build into a temporary table so a failure doesn't corrupt existing data.
    unordered_map<string, Course> tempTable;

    while (getline(infile2, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, ',')) {
            tokens.push_back(trim(token));
        }

        if (tokens.size() < 2) {
            // Already warned in first pass; skip bad line.
            continue;
        }

        Course course;
        course.courseNumber = tokens[0];
        course.name = tokens[1];

        // Remaining tokens are prerequisite courseNumbers (if any).
        for (size_t i = 2; i < tokens.size(); ++i) {
            const string& prereqId = tokens[i];
            if (prereqId.empty()) {
                continue;
            }

            // Only keep prerequisites that exist as courses in the file.
            if (courseIds.find(prereqId) != courseIds.end()) {
                course.prerequisites.push_back(prereqId);
            }
            else {
                cout << "Warning: Prerequisite '" << prereqId
                    << "' for course '" << course.courseNumber
                    << "' does not exist in the course list and will be ignored."
                    << endl;
            }
        }

        tempTable[course.courseNumber] = course;
    }

    infile2.close();

    // Loading was successful; replace the original table.
    courseTable = move(tempTable);

    cout << "Courses loaded successfully from '" << fileName << "'." << endl;
    cout << "Total courses loaded: " << courseTable.size() << endl;

    return true;
}

// -------------------- Printing Functions --------------------

// Print a single course and its prerequisites (course numbers only, matching sample output).
void printCourseInformation(const unordered_map<string, Course>& courseTable,
    const Course& course) {

    cout << course.courseNumber << ", " << course.name << endl;

    if (course.prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }

    cout << "Prerequisites: ";
    for (size_t i = 0; i < course.prerequisites.size(); ++i) {
        cout << course.prerequisites[i];
        if (i + 1 < course.prerequisites.size()) {
            cout << ", ";
        }
    }
    cout << endl;
}

// Print the full course list in alphanumeric order (by courseNumber).
void printCourseList(const unordered_map<string, Course>& courseTable) {
    if (courseTable.empty()) {
        cout << "Please load data structure first." << endl;
        return;
    }

    cout << "Here is a sample schedule:" << endl << endl;

    // Copy all Course objects into a vector so we can sort.
    vector<Course> courses;
    courses.reserve(courseTable.size());
    for (const auto& entry : courseTable) {
        courses.push_back(entry.second);
    }

    sort(courses.begin(), courses.end(),
        [](const Course& a, const Course& b) {
            return a.courseNumber < b.courseNumber;
        });

    for (const Course& course : courses) {
        cout << course.courseNumber << ", " << course.name << endl;
    }
}

// -------------------- Menu Functions --------------------

// Display the main menu options.
void displayMenu() {
    cout << "1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
}

// Handle printing a single course's information.
void handlePrintCourse(const unordered_map<string, Course>& courseTable) {
    if (courseTable.empty()) {
        cout << "Please load data structure first." << endl;
        return;
    }

    string courseNumber;
    cout << "What course do you want to know about? ";
    cin >> courseNumber;

    courseNumber = toUpper(courseNumber);

    auto it = courseTable.find(courseNumber);
    if (it == courseTable.end()) {
        cout << "Course " << courseNumber << " not found." << endl;
    }
    else {
        printCourseInformation(courseTable, it->second);
    }
}

// -------------------- Main Program --------------------
// Main menu loop for the advising assistance program.
int main() {
    unordered_map<string, Course> courseTable;
    bool dataLoaded = false;
    int choice = 0;

    cout << "Welcome to the course planner." << endl << endl;

    // Main loop for menu-driven interaction.
    while (true) {
        displayMenu();
        cout << "What would you like to do? ";

        // Validate numeric menu input.
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Input error: Please enter a number that matches one of the menu options." << endl;
            continue;
        }

        if (choice == 1) {
            // Load the course data from a CSV file.
            string fileName;
            cout << "Enter the file name to load: ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, fileName);
            fileName = trim(fileName);

            if (loadCourses(courseTable, fileName)) {
                dataLoaded = true;
            }
            else {
                cout << "Courses were not loaded. Please try again." << endl;
            }
        }
        else if (choice == 2) {
            // Print the course list in alphanumeric order.
            if (!dataLoaded) {
                cout << "Please load data structure first." << endl;
            }
            else {
                printCourseList(courseTable);
            }
        }
        else if (choice == 3) {
            // Print information about a single course.
            if (!dataLoaded) {
                cout << "Please load data structure first." << endl;
            }
            else {
                handlePrintCourse(courseTable);
            }
        }
        else if (choice == 9) {
            // Exit the program.
            cout << "Thank you for using the course planner!" << endl;
            break;
        }
        else {
            // Handle invalid numeric menu options like "8".
            cout << choice << " is not a valid option." << endl;
        }

        cout << endl; // Blank line between operations for readability.
    }

    return 0;
}