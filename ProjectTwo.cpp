// ============================================================
// ProjectTwo.cpp
// ABCU Advising Assistance Program
// Author: Daniel Cervantes
// Description: Loads course data from a CSV file into a Binary
//              Search Tree (BST), then allows advisors to print
//              an alphanumeric course list or look up individual
//              course information including prerequisites.
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// ============================================================
// Data Structures
// ============================================================

// Holds all information for a single course
struct Course {
    string courseNumber;          // "CSCI100"
    string courseName;            // "Intro to Computer Science"
    vector<string> prerequisites; // list of prerequisite course numbers
};

// A single node in the BST, keyed on courseNumber
struct Node {
    Course course;
    Node* left;
    Node* right;

    // Constructor
    Node(Course c) : course(c), left(nullptr), right(nullptr) {}
};

// ============================================================
// Binary Search Tree Class
// ============================================================

class CourseBST {
private:
    Node* root;

    // Helper: recursively insert a course into the subtree rooted at node
    Node* insertHelper(Node* node, Course course) {
        if (node == nullptr) {
            return new Node(course);
        }
        // BST ordering by courseNumber (case-insensitive comparison)
        string newKey = course.courseNumber;
        string curKey = node->course.courseNumber;
        transform(newKey.begin(), newKey.end(), newKey.begin(), ::toupper);
        transform(curKey.begin(), curKey.end(), curKey.begin(), ::toupper);

        if (newKey < curKey) {
            node->left = insertHelper(node->left, course);
        }
        else if (newKey > curKey) {
            node->right = insertHelper(node->right, course);
        }
        // Duplicate keys are ignored
        return node;
    }

    // Helper: in-order traversal prints courses in alphanumeric order
    void inOrderHelper(Node* node) {
        if (node == nullptr) {
            return;
        }
        inOrderHelper(node->left);
        cout << node->course.courseNumber << ", " << node->course.courseName << endl;
        inOrderHelper(node->right);
    }

    // Helper: search for a course by courseNumber
    Node* searchHelper(Node* node, string courseNumber) {
        if (node == nullptr) {
            return nullptr;
        }
        string searchKey = courseNumber;
        string curKey = node->course.courseNumber;
        transform(searchKey.begin(), searchKey.end(), searchKey.begin(), ::toupper);
        transform(curKey.begin(), curKey.end(), curKey.begin(), ::toupper);

        if (searchKey == curKey) {
            return node;
        }
        else if (searchKey < curKey) {
            return searchHelper(node->left, courseNumber);
        }
        else {
            return searchHelper(node->right, courseNumber);
        }
    }

    // Helper: recursively delete all nodes to free memory
    void destroyTree(Node* node) {
        if (node == nullptr) {
            return;
        }
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

public:
    // Constructor
    CourseBST() : root(nullptr) {}

    // Destructor — clean up all allocated nodes
    ~CourseBST() {
        destroyTree(root);
    }

    // Insert a course into the BST
    void insert(Course course) {
        root = insertHelper(root, course);
    }

    // Print all courses in alphanumeric (in-order) sequence
    void printCourseList() {
        inOrderHelper(root);
    }

    // Search for a course and return it; returns empty Course if not found
    Course search(string courseNumber) {
        Node* result = searchHelper(root, courseNumber);
        if (result != nullptr) {
            return result->course;
        }
        // Return an empty course to signal "not found"
        Course empty;
        return empty;
    }

    // Returns true if the BST contains at least one node
    bool isEmpty() {
        return root == nullptr;
    }
};

// ============================================================
// File Parsing
// ============================================================

// Trim leading/trailing whitespace from a string
string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    return str.substr(start, end - start + 1);
}

// Convert a string to uppercase in place
string toUpper(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// Load courses from a CSV file into the BST.
// Returns true on success, false if the file could not be opened.
// Two-pass approach:
//   Pass 1 — collect all valid course numbers
//   Pass 2 — build Course objects, skipping invalid prerequisites
bool loadCourses(const string& fileName, CourseBST& bst) {
    ifstream file(fileName);
    if (!file.is_open()) {
        cout << "Error: Could not open file '" << fileName << "'." << endl;
        return false;
    }

    // ---- Pass 1: collect all course numbers in the file ----
    vector<string> validCourseNumbers;
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        stringstream ss(line);
        string courseNumber;
        getline(ss, courseNumber, ',');
        courseNumber = trim(courseNumber);
        if (!courseNumber.empty()) {
            validCourseNumbers.push_back(toUpper(courseNumber));
        }
    }

    // ---- Pass 2: parse full records and insert into BST ----
    file.clear();
    file.seekg(0, ios::beg);

    int loadedCount = 0;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        vector<string> tokens;
        string token;
        while (getline(ss, token, ',')) {
            tokens.push_back(trim(token));
        }

        // Each valid line must have at least a course number and a name
        if (tokens.size() < 2) {
            cout << "Warning: Skipping malformed line: " << line << endl;
            continue;
        }

        Course course;
        course.courseNumber = toUpper(tokens[0]);
        course.courseName = tokens[1];

        // Validate and add prerequisites (index 2 onward)
        for (size_t i = 2; i < tokens.size(); ++i) {
            string prereq = toUpper(tokens[i]);
            if (prereq.empty()) {
                continue;
            }
            // Only add prerequisites that actually exist in the file
            bool found = false;
            for (const string& valid : validCourseNumbers) {
                if (valid == prereq) {
                    found = true;
                    break;
                }
            }
            if (found) {
                course.prerequisites.push_back(prereq);
            }
            else {
                cout << "Warning: Prerequisite '" << prereq
                    << "' for course '" << course.courseNumber
                    << "' not found in file. Skipping prerequisite." << endl;
            }
        }

        bst.insert(course);
        ++loadedCount;
    }

    file.close();
    cout << loadedCount << " courses loaded successfully." << endl;
    return true;
}

// ============================================================
// Menu Display
// ============================================================

void displayMenu() {
    cout << endl;
    cout << "   1. Load Data Structure." << endl;
    cout << "   2. Print Course List." << endl;
    cout << "   3. Print Course." << endl;
    cout << "   9. Exit" << endl;
    cout << endl;
}

// ============================================================
// Main
// ============================================================

int main() {
    CourseBST courseBST;  // The BST that will hold all course data
    int choice = 0;

    cout << "Welcome to the course planner." << endl;

    // Main menu loop
    while (choice != 9) {
        displayMenu();
        cout << "What would you like to do? ";

        // Validate that input is actually an integer
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        cin.ignore(1000, '\n'); // flush remaining input

        switch (choice) {
            // ---- Option 1: Load the data file ----
        case 1: {
            string fileName;
            cout << "Enter the file name: ";
            getline(cin, fileName);
            fileName = trim(fileName);
            loadCourses(fileName, courseBST);
            break;
        }

              // ---- Option 2: Print alphanumeric course list ----
        case 2: {
            if (courseBST.isEmpty()) {
                cout << "No data loaded. Please load a file first (Option 1)." << endl;
                break;
            }
            cout << "Here is a sample schedule:" << endl << endl;
            courseBST.printCourseList();
            break;
        }

              // ---- Option 3: Print individual course info ----
        case 3: {
            if (courseBST.isEmpty()) {
                cout << "No data loaded. Please load a file first (Option 1)." << endl;
                break;
            }
            string courseInput;
            cout << "What course do you want to know about? ";
            getline(cin, courseInput);
            courseInput = toUpper(trim(courseInput));

            Course found = courseBST.search(courseInput);
            if (found.courseNumber.empty()) {
                cout << "Course '" << courseInput << "' not found." << endl;
            }
            else {
                cout << found.courseNumber << ", " << found.courseName << endl;
                if (found.prerequisites.empty()) {
                    cout << "Prerequisites: None" << endl;
                }
                else {
                    cout << "Prerequisites: ";
                    for (size_t i = 0; i < found.prerequisites.size(); ++i) {
                        if (i > 0) {
                            cout << ", ";
                        }
                        cout << found.prerequisites[i];
                    }
                    cout << endl;
                }
            }
            break;
        }

              // ---- Option 9: Exit ----
        case 9:
            cout << "Thank you for using the course planner!" << endl;
            break;

            // ---- Any other input ----
        default:
            cout << choice << " is not a valid option." << endl;
            break;
        }
    }

    return 0;
}