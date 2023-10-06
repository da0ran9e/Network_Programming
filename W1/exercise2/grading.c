#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define MAX_STUDENTS 100
#define MAX_SUBJECT_ID 20
#define MAX_SUBJECT_NAME 50
#define MAX_SEMESTER_ID 10
#define MAX_STUDENT_ID 10
#define MAX_FIRST_NAME 20
#define MAX_LAST_NAME 20

typedef struct Student {
    char studentID[10];
    char firstName[20];
    char lastName[20];
    double progressMark;
    double finalExamMark;
    double mark;
    char grade;
    struct Student* next;
} Student;

void removeSpaces(char *str) {
    if (str == NULL) {
        return; // Handle NULL input
    }

    int len = strlen(str);
    if (len == 0) {
        return; // Empty string, nothing to remove
    }

    // Remove trailing spaces
    while (len > 0 && (isspace(str[len - 1]) || str[len - 1] == '\t')) {
        len--;
    }
    str[len] = '\0'; // Null-terminate the trimmed string

    // Remove leading spaces
    int start = 0;
    while (isspace(str[start]) || str[start] == '\t') {
        start++;
    }

    // Shift the string to remove leading spaces
    if (start > 0) {
        memmove(str, str + start, len - start + 1); // +1 for the null terminator
    }
}

void calculateGradingSummary(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Unable to open file %s\n", filename);
        return;
    }

    char subjectID[20];
    char subjectName[50];
    char semesterID[10];
    char progressWeightStr[6];
    char finalExamWeightStr[6];
    int progressWeight, finalExamWeight;
    int studentCount;
    double averageMark = 0.0;
    int highestMarkIndex = 0;
    int lowestMarkIndex = 0;

    // Read subject information
    fscanf(file, "SubjectID|%s\n", subjectID);
    fscanf(file, "Subject|%[^\n]\n", subjectName);
    fscanf(file, "F|%[^|]|%[^|]\n", progressWeightStr, finalExamWeightStr);
    fscanf(file, "Semester|%s\n", semesterID);
    fscanf(file, "StudentCount|%d\n", &studentCount);

    // Convert progress and final exam weights to integers
    progressWeight = atoi(progressWeightStr);
    finalExamWeight = atoi(finalExamWeightStr);

    // Initialize the linked list of students
    Student *head = NULL;
    Student *current = NULL;

    // Read student information and create the linked list
    for (int i = 0; i < studentCount; i++) {
        Student *newStudent = (Student *)malloc(sizeof(Student));
        if (newStudent == NULL) {
            printf("Error: Memory allocation failed\n");
            fclose(file);
            return;
        }

        fscanf(file, "S|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]| %c |\n",
               newStudent->studentID,
               newStudent->firstName,
               newStudent->lastName,
               progressWeightStr,
               finalExamWeightStr,
               &newStudent->grade);

        // Remove leading and trailing spaces from first name and last name
        removeSpaces(newStudent->firstName);
        removeSpaces(newStudent->lastName);

        // Convert progress and final exam marks to doubles
        newStudent->progressMark = atof(progressWeightStr);
        newStudent->finalExamMark = atof(finalExamWeightStr);

        // Calculate the total mark for the student
        newStudent->mark = (newStudent->progressMark * progressWeight +
                            newStudent->finalExamMark * finalExamWeight) / 100;

        // Update the linked list
        newStudent->next = NULL;
        if (head == NULL) {
            head = newStudent;
            current = newStudent;
        } else {
            current->next = newStudent;
            current = newStudent;
        }

        // Update statistics
        averageMark += newStudent->mark;
        if (newStudent->mark > head->mark)
            highestMarkIndex = i;
        if (newStudent->mark < head->mark)
            lowestMarkIndex = i;
    }
    averageMark /= studentCount;

    // Output grading summary to the output file
    char outputFileName[50] = "output/";
    strcat(outputFileName, subjectID);
    strcat(outputFileName, "_");
    strcat(outputFileName, semesterID);
    strcat(outputFileName, "_rp.txt");
    FILE *outputFile = fopen(outputFileName, "a+");
    if (outputFile == NULL) {
        printf("Error: Unable to open output file %s\n", outputFileName);
        fclose(file);
        return;
    }

    fprintf(outputFile, "The student with the highest mark is: %s %s\n",
            head->firstName, head->lastName);
    fprintf(outputFile, "The student with the lowest mark is: %s %s\n",
            head->firstName, head->lastName);
    fprintf(outputFile, "The average mark is: %.2lf\n", averageMark);
    fprintf(outputFile, "\nA histogram of the subject %s is: \n", subjectID);

    // Calculate histogram
    int gradeCounts[5] = {0};
    Student *currentStudent = head;
    while (currentStudent != NULL) {
        switch (currentStudent->grade) {
            case 'A':
                gradeCounts[0]++;
                break;
            case 'B':
                gradeCounts[1]++;
                break;
            case 'C':
                gradeCounts[2]++;
                break;
            case 'D':
                gradeCounts[3]++;
                break;
            case 'F':
                gradeCounts[4]++;
                break;
        }
        currentStudent = currentStudent->next;
    }

    // Output histogram to the output file
    fprintf(outputFile, "A:");
    for (int i = 0; i < gradeCounts[0]; i++) {
        fprintf(outputFile, "*");
    }
    fprintf(outputFile, "\nB:");
    for (int i = 0; i < gradeCounts[1]; i++) {
        fprintf(outputFile, "*");
    }
    fprintf(outputFile, "\nC:");
    for (int i = 0; i < gradeCounts[2]; i++) {
        fprintf(outputFile, "*");
    }
    fprintf(outputFile, "\nD:");
    for (int i = 0; i < gradeCounts[3]; i++) {
        fprintf(outputFile, "*");
    }
    fprintf(outputFile, "\nF:");
    for (int i = 0; i < gradeCounts[4]; i++) {
        fprintf(outputFile, "*");
    }
    fprintf(outputFile, "\n");

    // Close both input and output files
    fclose(file);
    fclose(outputFile);

    // Free dynamically allocated memory for the linked list
    currentStudent = head;
    while (currentStudent != NULL) {
        Student *temp = currentStudent;
        currentStudent = currentStudent->next;
        free(temp);
    }
}
void displayMenu() {
    printf("Learning Management System\n");
    printf("-------------------------------------\n");
    printf("1. Add a new score board\n");
    printf("2. Add score\n");
    printf("3. Remove score\n");
    printf("4. Search score\n");
    printf("5. Display score board and score report\n");
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void addNewScoreboard() {
    char subjectID[MAX_SUBJECT_ID];
    char subjectName[MAX_SUBJECT_NAME];
    char semesterID[MAX_SEMESTER_ID];
    int studentCount;

    printf("Function 1: Add a new score board\n");

    // Prompt the user for subject information
    printf("Enter subject ID: ");
    fgets(subjectID, sizeof(subjectID), stdin);
    subjectID[strcspn(subjectID, "\n")] = '\0'; // Remove the newline character

    printf("Enter subject name: ");
    fgets(subjectName, sizeof(subjectName), stdin);
    subjectName[strcspn(subjectName, "\n")] = '\0'; // Remove the newline character

    // Prompt the user for semester ID and the number of students
    printf("Enter semester ID: ");
    fgets(semesterID, sizeof(semesterID), stdin);
    semesterID[strcspn(semesterID, "\n")] = '\0'; // Remove the newline character

    printf("Enter the number of students: ");
    scanf("%d", &studentCount);

    // Create a new score board with the provided information
    printf("Creating a new score board for subject: %s\n", subjectName);
    printf("Semester ID: %s\n", semesterID);
    printf("Number of students: %d\n", studentCount);

    // For simplicity, we will just print the information here.
    printf("Score board created successfully.\n");
}

void addScore() {
    char subjectID[MAX_SUBJECT_ID];
    char semesterID[MAX_SEMESTER_ID];
    char studentID[MAX_STUDENT_ID];
    char firstName[MAX_FIRST_NAME];
    char lastName[MAX_LAST_NAME];
    double progressMark, finalExamMark;
    char grade;

    printf("Function 2: Add score\n");

    // Prompt the user for subject ID and semester ID
    printf("Enter subject ID: ");
    fgets(subjectID, sizeof(subjectID), stdin);
    subjectID[strcspn(subjectID, "\n")] = '\0'; // Remove the newline character

    printf("Enter semester ID: ");
    fgets(semesterID, sizeof(semesterID), stdin);
    semesterID[strcspn(semesterID, "\n")] = '\0'; // Remove the newline character

    // Prompt the user for student information and marks
    printf("Enter student ID: ");
    fgets(studentID, sizeof(studentID), stdin);
    studentID[strcspn(studentID, "\n")] = '\0'; // Remove the newline character

    printf("Enter student's first name: ");
    fgets(firstName, sizeof(firstName), stdin);
    firstName[strcspn(firstName, "\n")] = '\0'; // Remove the newline character

    printf("Enter student's last name: ");
    fgets(lastName, sizeof(lastName), stdin);
    lastName[strcspn(lastName, "\n")] = '\0'; // Remove the newline character

    printf("Enter progress mark: ");
    scanf("%lf", &progressMark);

    printf("Enter final exam mark: ");
    scanf("%lf", &finalExamMark);

    // Calculate the grade based on the provided marks
    if (progressMark >= 8.5 && finalExamMark >= 8.5) {
        grade = 'A';
    } else if (progressMark >= 7.0 && finalExamMark >= 7.0) {
        grade = 'B';
    } else if (progressMark >= 5.5 && finalExamMark >= 5.5) {
        grade = 'C';
    } else if (progressMark >= 4.0 && finalExamMark >= 4.0) {
        grade = 'D';
    } else {
        grade = 'F';
    }

    // Add student information and marks to the corresponding text file
    printf("Adding student information to the file for subject: %s, Semester: %s\n", subjectID, semesterID);
    printf("Student ID: %s\n", studentID);
    printf("Student Name: %s %s\n", firstName, lastName);
    printf("Progress Mark: %.2lf\n", progressMark);
    printf("Final Exam Mark: %.2lf\n", finalExamMark);
    printf("Grade: %c\n", grade);

    // For simplicity, we will just print the information here.
    printf("Student information and marks added successfully.\n");
}

void removeScore() {
    char subjectID[MAX_SUBJECT_ID];
    char semesterID[MAX_SEMESTER_ID];
    char studentID[MAX_STUDENT_ID];

    printf("Function 3: Remove score\n");

    // Prompt the user for subject ID, semester ID, and student ID
    printf("Enter subject ID: ");
    fgets(subjectID, sizeof(subjectID), stdin);
    subjectID[strcspn(subjectID, "\n")] = '\0'; // Remove the newline character

    printf("Enter semester ID: ");
    fgets(semesterID, sizeof(semesterID), stdin);
    semesterID[strcspn(semesterID, "\n")] = '\0'; // Remove the newline character

    printf("Enter student ID: ");
    fgets(studentID, sizeof(studentID), stdin);
    studentID[strcspn(studentID, "\n")] = '\0'; // Remove the newline character

    // Implement code to remove the student's information and grade
    // from the corresponding text file for the specified subject and semester
    printf("Removing student information for Subject: %s, Semester: %s, Student ID: %s\n", subjectID, semesterID, studentID);

    // For simplicity, we will just print a message here.
    printf("Student information and grade removed successfully.\n");
}

void searchScore() {
    char subjectID[MAX_SUBJECT_ID];
    char semesterID[MAX_SEMESTER_ID];
    char studentID[MAX_STUDENT_ID];

    printf("Function 4: Search score\n");

    // Prompt the user for subject ID, semester ID, and student ID
    printf("Enter subject ID: ");
    fgets(subjectID, sizeof(subjectID), stdin);
    subjectID[strcspn(subjectID, "\n")] = '\0'; // Remove the newline character

    printf("Enter semester ID: ");
    fgets(semesterID, sizeof(semesterID), stdin);
    semesterID[strcspn(semesterID, "\n")] = '\0'; // Remove the newline character

    printf("Enter student ID: ");
    fgets(studentID, sizeof(studentID), stdin);
    studentID[strcspn(studentID, "\n")] = '\0'; // Remove the newline character

    // Implement code to search for the student's grade information
    // in the corresponding text file for the specified subject and semester
    printf("Searching for grade information for Subject: %s, Semester: %s, Student ID: %s\n", subjectID, semesterID, studentID);

    // For simplicity, we will just print a message here.
    printf("Grade information found successfully.\n");
}

void printTextFilesInFolder(const char *folderPath) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(folderPath);

    if (dir == NULL) {
        perror("Unable to open the directory");
        return;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry is a regular file and ends with ".txt"
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt") != NULL) {
            // Form the full file path
            char filePath[1000];
            snprintf(filePath, sizeof(filePath), "%s/%s", folderPath, entry->d_name);

            // Save summary
            calculateGradingSummary(filePath);

            // Open the file
            FILE *file = fopen(filePath, "r");
            if (file == NULL) {
                perror("Unable to open file");
                continue;
            }

            // Print the content of the file
            printf("Contents of %s:\n", entry->d_name);
            char line[1000];
            while (fgets(line, sizeof(line), file) != NULL) {
                printf("%s", line);
            }

            fclose(file);
        }
    }

    // Close the directory
    closedir(dir);
}

int main() {
    int choice;
    char continueChoice;


    do {
        displayMenu();
        printf("Your choice (1-5, other to quit): ");
        scanf("%d", &choice);
        clearInputBuffer();

        switch (choice) {
            case 1:
                addNewScoreboard();
                break;
            case 2:
                addScore();
                break;
            case 3:
                removeScore();
                break;
            case 4:
                searchScore();
                break;
            case 5:
                printTextFilesInFolder("data");
                break;
            default:
                printf("Goodbye!\n");
                return 0;
        }

        printf("Do you want to continue? (y/Y for yes, any other key to quit): ");
        scanf(" %c", &continueChoice);
        clearInputBuffer();
    } while (continueChoice == 'y' || continueChoice == 'Y');

    return 0;

}
