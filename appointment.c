#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN 50
#define MAX_PASS_LEN 50
#define ENCRYPTION_KEY 3
#define START_HOUR 9
#define END_HOUR 17
#define APPOINTMENT_DURATION 30 // minutes
#define MAX_APPOINTMENTS_PER_DAY ((END_HOUR - START_HOUR) * 60 / APPOINTMENT_DURATION)
#define DATA_FILE "appointments.dat"
#define USER_FILE "users.dat"

// Date structure to track appointments across multiple days
typedef struct date {
    int day;
    int month;
    int year;
} Date;

// Node structure for appointment
typedef struct appointment {
    char name[MAX_NAME_LEN];
    char illness[MAX_NAME_LEN];
    int hour;
    int minute;
    Date date;
    struct appointment* next;
} Appointment;

// Structure for user authentication
typedef struct user {
    char username[MAX_NAME_LEN];
    char password[MAX_PASS_LEN];
    int is_admin; // Flag to denote admin privileges
    struct user* next;
} User;

// Function prototypes
void clearInputBuffer();
void encrypt(char* str);
void decrypt(char* str);
Appointment* createAppointment();
void addAppointment(Appointment** head);
void deleteAppointment(Appointment** head);
void displayAppointments(Appointment* head);
void searchAppointmentByName(Appointment* head);
void modifyAppointment(Appointment** head);
void saveAppointmentsToFile(Appointment* head);
void loadAppointmentsFromFile(Appointment** head);
User* createUser();
void addUser(User** head);
int authenticateUser(User* head, char* username, char* password, int* is_admin);
void saveUsersToFile(User* head);
void loadUsersFromFile(User** head);
void adminMenu(Appointment** appointmentList, User** userList);
void displayAvailableSlots(Appointment* head, Date date);
void freeAppointmentList(Appointment** head);
void freeUserList(User** head);
Date getDate();
int isDateValid(Date date);
int compareDate(Date date1, Date date2);
int isSlotAvailable(Appointment* head, int hour, int minute, Date date);

int main() {
    Appointment* appointmentList = NULL;
    User* userList = NULL;
    int choice, is_admin = 0;
    char username[MAX_NAME_LEN], password[MAX_PASS_LEN];
    
    // Load existing data
    loadAppointmentsFromFile(&appointmentList);
    loadUsersFromFile(&userList);
    
    // If no users exist, create an admin account
    if (userList == NULL) {
        printf("No users found. Creating admin account.\n");
        User* admin = (User*)malloc(sizeof(User));
        if (admin == NULL) {
            printf("Memory allocation failed.\n");
            return 1;
        }
        
        strcpy(admin->username, "admin");
        strcpy(admin->password, "admin123");
        admin->is_admin = 1;
        admin->next = NULL;
        userList = admin;
        saveUsersToFile(userList);
        printf("Admin account created. Username: admin, Password: admin123\n");
    }
    
    while (1) {
        printf("\n===== APPOINTMENT SYSTEM =====\n");
        printf("1. Sign Up\n");
        printf("2. Sign In\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        
        switch (choice) {
            case 1:
                addUser(&userList);
                saveUsersToFile(userList);
                break;
                
            case 2:
                printf("Enter username: ");
                scanf("%s", username);
                
                printf("Enter password: ");
                scanf("%s", password);
                
                if (authenticateUser(userList, username, password, &is_admin)) {
                    printf("Login successful!\n");
                    
                    if (is_admin) {
                        adminMenu(&appointmentList, &userList);
                    } else {
                        // Regular user menu
                        while (1) {
                            printf("\n===== PATIENT MENU =====\n");
                            printf("1. Book an appointment\n");
                            printf("2. View my appointments\n");
                            printf("3. Cancel an appointment\n");
                            printf("4. Modify an appointment\n");
                            printf("5. View available slots\n");
                            printf("6. Log out\n");
                            printf("Enter your choice: ");
                            
                            if (scanf("%d", &choice) != 1) {
                                printf("Invalid input. Please enter a number.\n");
                                clearInputBuffer();
                                continue;
                            }
                            
                            switch (choice) {
                                case 1:
                                    addAppointment(&appointmentList);
                                    saveAppointmentsToFile(appointmentList);
                                    break;
                                case 2:
                                    displayAppointments(appointmentList);
                                    break;
                                case 3:
                                    deleteAppointment(&appointmentList);
                                    saveAppointmentsToFile(appointmentList);
                                    break;
                                case 4:
                                    modifyAppointment(&appointmentList);
                                    saveAppointmentsToFile(appointmentList);
                                    break;
                                case 5:
                                    {
                                        Date date = getDate();
                                        if (isDateValid(date)) {
                                            displayAvailableSlots(appointmentList, date);
                                        }
                                    }
                                    break;
                                case 6:
                                    printf("Logging out...\n");
                                    break;
                                default:
                                    printf("Invalid choice. Please try again.\n");
                            }
                            
                            if (choice == 6) break;
                        }
                    }
                } else {
                    printf("Login failed. Incorrect username or password.\n");
                }
                break;
                
            case 3:
                printf("Thank you for using the Appointment System.\n");
                // Free memory before exiting
                freeAppointmentList(&appointmentList);
                freeUserList(&userList);
                return 0;
                
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    
    return 0;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void encrypt(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = str[i] + ENCRYPTION_KEY;
    }
}

void decrypt(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = str[i] - ENCRYPTION_KEY;
    }
}

Appointment* createAppointment() {
    Appointment* newAppointment = (Appointment*)malloc(sizeof(Appointment));
    if (newAppointment == NULL) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Enter your name: ");
    scanf("%s", newAppointment->name);
    encrypt(newAppointment->name);
    
    printf("Enter what you are suffering from: ");
    scanf("%s", newAppointment->illness);
    encrypt(newAppointment->illness);
    
    // Get date for the appointment
    newAppointment->date = getDate();
    if (!isDateValid(newAppointment->date)) {
        free(newAppointment);
        return NULL;
    }
    
    newAppointment->next = NULL;
    return newAppointment;
}

void addAppointment(Appointment** head) {
    Appointment* newAppointment = createAppointment();
    if (newAppointment == NULL) return;
    
    // Show available slots
    printf("\nAvailable slots for the selected date:\n");
    displayAvailableSlots(*head, newAppointment->date);
    
    // Get time slot from user
    int hour, minute, validSlot = 0;
    
    while (!validSlot) {
        printf("Enter preferred hour (9-17): ");
        scanf("%d", &hour);
        
        printf("Enter preferred minute (0 or 30): ");
        scanf("%d", &minute);
        
        if (hour < START_HOUR || hour > END_HOUR || (minute != 0 && minute != 30)) {
            printf("Invalid time slot. Please choose a time between 9:00 and 17:30 (30-minute intervals).\n");
            continue;
        }
        
        if (isSlotAvailable(*head, hour, minute, newAppointment->date)) {
            validSlot = 1;
        } else {
            printf("The selected slot is already booked. Please choose another time.\n");
        }
    }
    
    newAppointment->hour = hour;
    newAppointment->minute = minute;
    
    // Add appointment to list (sorted by date and time)
    if (*head == NULL || 
        (compareDate(newAppointment->date, (*head)->date) < 0) || 
        (compareDate(newAppointment->date, (*head)->date) == 0 && 
         (newAppointment->hour < (*head)->hour || 
          (newAppointment->hour == (*head)->hour && newAppointment->minute < (*head)->minute)))) {
        newAppointment->next = *head;
        *head = newAppointment;
    } else {
        Appointment* current = *head;
        
        while (current->next != NULL && 
              (compareDate(newAppointment->date, current->next->date) > 0 || 
              (compareDate(newAppointment->date, current->next->date) == 0 && 
               (newAppointment->hour > current->next->hour || 
                (newAppointment->hour == current->next->hour && newAppointment->minute >= current->next->minute))))) {
            current = current->next;
        }
        
        newAppointment->next = current->next;
        current->next = newAppointment;
    }
    
    printf("Your appointment has been successfully booked for %02d/%02d/%04d at %02d:%02d\n", 
           newAppointment->date.day, newAppointment->date.month, newAppointment->date.year,
           newAppointment->hour, newAppointment->minute);
}

void deleteAppointment(Appointment** head) {
    if (*head == NULL) {
        printf("No appointments to delete.\n");
        return;
    }
    
    displayAppointments(*head);
    
    int day, month, year, hour, minute;
    printf("Enter the date (DD MM YYYY) of the appointment to delete: ");
    scanf("%d %d %d", &day, &month, &year);
    
    printf("Enter the time (HH MM) of the appointment to delete: ");
    scanf("%d %d", &hour, &minute);
    
    Date date = {day, month, year};
    
    // Confirm deletion
    printf("Are you sure you want to delete this appointment? (1 for Yes, 0 for No): ");
    int confirm;
    scanf("%d", &confirm);
    
    if (!confirm) {
        printf("Deletion cancelled.\n");
        return;
    }
    
    Appointment* current = *head;
    Appointment* previous = NULL;
    
    while (current != NULL) {
        if (compareDate(current->date, date) == 0 && 
            current->hour == hour && current->minute == minute) {
            
            if (previous == NULL) {
                *head = current->next;
            } else {
                previous->next = current->next;
            }
            
            free(current);
            printf("Appointment successfully deleted.\n");
            return;
        }
        
        previous = current;
        current = current->next;
    }
    
    printf("Appointment not found.\n");
}

void displayAppointments(Appointment* head) {
    if (head == NULL) {
        printf("No appointments scheduled.\n");
        return;
    }
    
    printf("\n===== CURRENT APPOINTMENTS =====\n");
    printf("%-20s %-20s %-12s %-10s\n", "Name", "Illness", "Date", "Time");
    printf("---------------------------------------------------------------\n");
    
    Appointment* current = head;
    char decryptedName[MAX_NAME_LEN];
    char decryptedIllness[MAX_NAME_LEN];
    
    while (current != NULL) {
        strcpy(decryptedName, current->name);
        strcpy(decryptedIllness, current->illness);
        
        decrypt(decryptedName);
        decrypt(decryptedIllness);
        
        printf("%-20s %-20s %02d/%02d/%04d  %02d:%02d\n", 
               decryptedName, decryptedIllness, 
               current->date.day, current->date.month, current->date.year,
               current->hour, current->minute);
        
        current = current->next;
    }
}

void searchAppointmentByName(Appointment* head) {
    if (head == NULL) {
        printf("No appointments to search.\n");
        return;
    }
    
    char searchName[MAX_NAME_LEN];
    printf("Enter the name to search for: ");
    scanf("%s", searchName);
    
    encrypt(searchName); // Encrypt to match stored data
    
    Appointment* current = head;
    int found = 0;
    
    printf("\n===== SEARCH RESULTS =====\n");
    
    while (current != NULL) {
        if (strcmp(current->name, searchName) == 0) {
            char decryptedName[MAX_NAME_LEN];
            char decryptedIllness[MAX_NAME_LEN];
            
            strcpy(decryptedName, current->name);
            strcpy(decryptedIllness, current->illness);
            
            decrypt(decryptedName);
            decrypt(decryptedIllness);
            
            printf("Name: %s\n", decryptedName);
            printf("Illness: %s\n", decryptedIllness);
            printf("Date: %02d/%02d/%04d\n", 
                   current->date.day, current->date.month, current->date.year);
            printf("Time: %02d:%02d\n\n", current->hour, current->minute);
            
            found = 1;
        }
        
        current = current->next;
    }
    
    if (!found) {
        printf("No appointments found for '%s'.\n", searchName);
    }
}

void modifyAppointment(Appointment** head) {
    if (*head == NULL) {
        printf("No appointments to modify.\n");
        return;
    }
    
    displayAppointments(*head);
    
    int day, month, year, hour, minute;
    printf("Enter the date (DD MM YYYY) of the appointment to modify: ");
    scanf("%d %d %d", &day, &month, &year);
    
    printf("Enter the time (HH MM) of the appointment to modify: ");
    scanf("%d %d", &hour, &minute);
    
    Date date = {day, month, year};
    
    Appointment* current = *head;
    
    while (current != NULL) {
        if (compareDate(current->date, date) == 0 && 
            current->hour == hour && current->minute == minute) {
            
            printf("\nCurrent appointment details:\n");
            char decryptedName[MAX_NAME_LEN], decryptedIllness[MAX_NAME_LEN];
            strcpy(decryptedName, current->name);
            strcpy(decryptedIllness, current->illness);
            decrypt(decryptedName);
            decrypt(decryptedIllness);
            
            printf("Name: %s\n", decryptedName);
            printf("Illness: %s\n", decryptedIllness);
            printf("Date: %02d/%02d/%04d\n", 
                   current->date.day, current->date.month, current->date.year);
            printf("Time: %02d:%02d\n", current->hour, current->minute);
            
            printf("\nWhat would you like to modify?\n");
            printf("1. Date and time\n");
            printf("2. Illness details\n");
            printf("3. Cancel modification\n");
            
            int choice;
            printf("Enter your choice: ");
            scanf("%d", &choice);
            
            switch (choice) {
                case 1:
                    printf("Enter new date (DD MM YYYY): ");
                    scanf("%d %d %d", &day, &month, &year);
                    current->date.day = day;
                    current->date.month = month;
                    current->date.year = year;
                    
                    printf("Enter new time (HH MM): ");
                    scanf("%d %d", &hour, &minute);
                    current->hour = hour;
                    current->minute = minute;
                    
                    printf("Appointment rescheduled successfully.\n");
                    break;
                    
                case 2:
                    printf("Enter new illness details: ");
                    char newIllness[MAX_NAME_LEN];
                    scanf("%s", newIllness);
                    encrypt(newIllness);
                    strcpy(current->illness, newIllness);
                    printf("Illness details updated successfully.\n");
                    break;
                    
                case 3:
                    printf("Modification cancelled.\n");
                    break;
                    
                default:
                    printf("Invalid choice.\n");
            }
            
            return;
        }
        
        current = current->next;
    }
    
    printf("Appointment not found.\n");
}

void saveAppointmentsToFile(Appointment* head) {
    FILE* file = fopen(DATA_FILE, "wb");
    
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }
    
    Appointment* current = head;
    
    while (current != NULL) {
        fwrite(current, sizeof(Appointment), 1, file);
        current = current->next;
    }
    
    fclose(file);
}

void loadAppointmentsFromFile(Appointment** head) {
    FILE* file = fopen(DATA_FILE, "rb");
    
    if (file == NULL) {
        // File doesn't exist yet, not an error
        return;
    }
    
    // Free existing list
    freeAppointmentList(head);
    
    Appointment temp;
    
    while (fread(&temp, sizeof(Appointment), 1, file) == 1) {
        Appointment* newAppointment = (Appointment*)malloc(sizeof(Appointment));
        
        if (newAppointment == NULL) {
            printf("Memory allocation failed while loading appointments.\n");
            fclose(file);
            return;
        }
        
        *newAppointment = temp;
        newAppointment->next = NULL;
        
        // Insert in sorted order
        if (*head == NULL || 
            (compareDate(newAppointment->date, (*head)->date) < 0) ||
            (compareDate(newAppointment->date, (*head)->date) == 0 && 
             (newAppointment->hour < (*head)->hour || 
              (newAppointment->hour == (*head)->hour && newAppointment->minute < (*head)->minute)))) {
            
            newAppointment->next = *head;
            *head = newAppointment;
        } else {
            Appointment* current = *head;
            
            while (current->next != NULL && 
                  (compareDate(newAppointment->date, current->next->date) > 0 ||
                  (compareDate(newAppointment->date, current->next->date) == 0 && 
                   (newAppointment->hour > current->next->hour || 
                    (newAppointment->hour == current->next->hour && newAppointment->minute >= current->next->minute))))) {
                current = current->next;
            }
            
            newAppointment->next = current->next;
            current->next = newAppointment;
        }
    }
    
    fclose(file);
}

User* createUser() {
    User* newUser = (User*)malloc(sizeof(User));
    
    if (newUser == NULL) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Enter username: ");
    scanf("%s", newUser->username);
    
    char password[MAX_PASS_LEN], confirmPassword[MAX_PASS_LEN];
    int matchingPasswords = 0;
    int attempts = 3;
    
    while (!matchingPasswords && attempts > 0) {
        printf("Enter password: ");
        scanf("%s", password);
        
        printf("Confirm password: ");
        scanf("%s", confirmPassword);
        
        if (strcmp(password, confirmPassword) == 0) {
            strcpy(newUser->password, password);
            matchingPasswords = 1;
        } else {
            attempts--;
            printf("Passwords do not match. You have %d attempts left.\n", attempts);
        }
    }
    
    if (!matchingPasswords) {
        printf("Failed to create account due to password mismatch.\n");
        free(newUser);
        return NULL;
    }
    
    newUser->is_admin = 0; // Default to regular user
    newUser->next = NULL;
    
    return newUser;
}

void addUser(User** head) {
    User* newUser = createUser();
    
    if (newUser == NULL) return;
    
    // Check if username already exists
    User* current = *head;
    
    while (current != NULL) {
        if (strcmp(current->username, newUser->username) == 0) {
            printf("Username already exists. Please choose a different username.\n");
            free(newUser);
            return;
        }
        current = current->next;
    }
    
    // Add to the beginning of the list
    newUser->next = *head;
    *head = newUser;
    
    printf("User account created successfully.\n");
}

int authenticateUser(User* head, char* username, char* password, int* is_admin) {
    User* current = head;
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && 
            strcmp(current->password, password) == 0) {
            *is_admin = current->is_admin;
            return 1; // Authentication successful
        }
        
        current = current->next;
    }
    
    return 0; // Authentication failed
}

void saveUsersToFile(User* head) {
    FILE* file = fopen(USER_FILE, "wb");
    
    if (file == NULL) {
        printf("Error opening user file for writing.\n");
        return;
    }
    
    User* current = head;
    
    while (current != NULL) {
        fwrite(current, sizeof(User), 1, file);
        current = current->next;
    }
    
    fclose(file);
}

void loadUsersFromFile(User** head) {
    FILE* file = fopen(USER_FILE, "rb");
    
    if (file == NULL) {
        // File doesn't exist yet, not an error
        return;
    }
    
    // Free existing list
    freeUserList(head);
    
    User temp;
    
    while (fread(&temp, sizeof(User), 1, file) == 1) {
        User* newUser = (User*)malloc(sizeof(User));
        
        if (newUser == NULL) {
            printf("Memory allocation failed while loading users.\n");
            fclose(file);
            return;
        }
        
        *newUser = temp;
        newUser->next = NULL;
        
        // Add to the beginning of the list
        newUser->next = *head;
        *head = newUser;
    }
    
    fclose(file);
}

void adminMenu(Appointment** appointmentList, User** userList) {
    int choice;
    
    while (1) {
        printf("\n===== ADMIN MENU =====\n");
        printf("1. View all appointments\n");
        printf("2. Search appointments by name\n");
        printf("3. Delete an appointment\n");
        printf("4. Create a new user\n");
        printf("5. Log out\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        
        switch (choice) {
            case 1:
                displayAppointments(*appointmentList);
                break;
            case 2:
                searchAppointmentByName(*appointmentList);
                break;
            case 3:
                deleteAppointment(appointmentList);
                saveAppointmentsToFile(*appointmentList);
                break;
            case 4:
                addUser(userList);
                saveUsersToFile(*userList);
                break;
            case 5:
                printf("Logging out from admin account...\n");
                return;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

void displayAvailableSlots(Appointment* head, Date date) {
    printf("\n===== AVAILABLE SLOTS FOR %02d/%02d/%04d =====\n", 
           date.day, date.month, date.year);
    
    int slotsAvailable = 0;
    
    for (int hour = START_HOUR; hour <= END_HOUR; hour++) {
        for (int minute = 0; minute < 60; minute += APPOINTMENT_DURATION) {
            // Skip 17:30 as it's outside office hours
            if (hour == END_HOUR && minute > 0) continue;
            
            if (isSlotAvailable(head, hour, minute, date)) {
                printf("%02d:%02d\n", hour, minute);
                slotsAvailable++;
            }
        }
    }
    
    if (slotsAvailable == 0) {
        printf("No available slots for this date.\n");
    }
}

int isSlotAvailable(Appointment* head, int hour, int minute, Date date) {
    Appointment* current = head;
    
    while (current != NULL) {
        if (compareDate(current->date, date) == 0 && 
            current->hour == hour && current->minute == minute) {
            return 0; // Slot is already booked
        }
        current = current->next;
    }
    
    return 1; // Slot is available
}

void freeAppointmentList(Appointment** head) {
    Appointment* current = *head;
    Appointment* next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    *head = NULL;
}

void freeUserList(User** head) {
    User* current = *head;
    User* next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    *head = NULL;
}

Date getDate() {
    Date date;
    
    printf("Enter date (DD MM YYYY): ");
    scanf("%d %d %d", &date.day, &date.month, &date.year);
    
    return date;
}

int isDateValid(Date date) {
    // Basic validation
    if (date.month < 1 || date.month > 12) {
        printf("Invalid month. Please enter a month between 1 and 12.\n");
        return 0;
    }
    
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Check for leap year
    if (date.month == 2 && ((date.year % 4 == 0 && date.year % 100 != 0) || date.year % 400 == 0)) {
        daysInMonth[2] = 29;
    }
    
    if (date.day < 1 || date.day > daysInMonth[date.month]) {
        printf("Invalid day for the given month.\n");
        return 0;
    }
    
    // Get current date for comparison
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    
    Date today = {
        .day = local->tm_mday,
        .month = local->tm_mon + 1,  // tm_mon is 0-based
        .year = local->tm_year + 1900  // tm_year is years since 1900
    };
    
    // Check if date is in the past
    if (compareDate(date, today) < 0) {
        printf("Cannot book appointments for past dates.\n");
        return 0;
    }
    
    return 1;
}

int compareDate(Date date1, Date date2) {
    if (date1.year < date2.year) return -1;
    if (date1.year > date2.year) return 1;
    
    if (date1.month < date2.month) return -1;
    if (date1.month > date2.month) return 1;
    
    if (date1.day < date2.day) return -1;
    if (date1.day > date2.day) return 1;
    
    return 0; // Dates are equal
}