//
//  main.c
//  Process-Creation-Hierarchy
//
//  Created by Behrouz Barati on 2/1/24.
//

#include <stdio.h>
#include <stdlib.h>

#define MAX_PCB_SIZE 100
#define PCB_TABLE_HEADER "i\tParent\tFirst\tOlder\tYounger\n---------------------------------------\n"
#define MENU_OF_OPTIONS "1) Enter parameters\n2) Create a new child process\n3) Destroy all descendants of a process\n4) Quit program and free memory\nEnter selection: "


struct node {
    int *parent;
    int *firstChild;
    int *olderSibling;
    int *youngerSibling;
}*pcb = NULL;
typedef struct node pcb_type;

int numberOfProcesses = 0;

void printPCBTable(void) {
    printf("\n");
    printf(PCB_TABLE_HEADER);
    for (int i = 0; i < numberOfProcesses; i++) {
        // Check if the process has not been initialized or destroyed
        if (pcb[i].parent == NULL || *pcb[i].parent == -1) {
            // Skip printing this row
            continue;
        }

        printf("%d\t", i);
        
        if (pcb[i].parent && *pcb[i].parent != -1) {
            printf("%d\t", *pcb[i].parent);
        } else printf("\t");

        if (pcb[i].firstChild && *pcb[i].firstChild != -1) {
            printf("\t%d\t", *pcb[i].firstChild);
        } else printf("\t\t");

        if (pcb[i].olderSibling && *pcb[i].olderSibling != -1) {
            printf("\t%d\t", *pcb[i].olderSibling);
        } else printf("\t\t");
        
        if (pcb[i].youngerSibling && *pcb[i].youngerSibling != -1) {
            printf("\t%d", *pcb[i].youngerSibling);
        } else printf("\t\t");
        
        printf("\n");
    }
    printf("\n");
}

void initializePCBArray(int maxProcessesNumber) {
    pcb = (pcb_type *)malloc(maxProcessesNumber * sizeof(pcb_type));
    for (int i = 0; i < maxProcessesNumber; i++) {
        pcb[i].parent = (int *)malloc(sizeof(int));
        pcb[i].firstChild = (int *)malloc(sizeof(int));
        pcb[i].olderSibling = (int *)malloc(sizeof(int));
        pcb[i].youngerSibling = (int *)malloc(sizeof(int));
        *pcb[i].parent = -1;
        *pcb[i].firstChild = -1;
        *pcb[i].olderSibling = -1;
        *pcb[i].youngerSibling = -1;
    }
    numberOfProcesses = maxProcessesNumber;
}

void freePCBArray(void) {
    if (pcb != NULL) {
        for (int i = 0; i < numberOfProcesses; i++) {
            free(pcb[i].parent);
            free(pcb[i].firstChild);
            free(pcb[i].olderSibling);
            free(pcb[i].youngerSibling);
        }
        free(pcb);
        pcb = NULL;
    }
}

void option1(void) {
    int maxProcessesNumber;
    printf("Enter the maximum number of processes: ");
    while (scanf("%d", &maxProcessesNumber) != 1 || maxProcessesNumber <= 0) {
        printf("Error: Invalid input. Enter a positive integer for the maximum number of processes: ");
        // Clear the input buffer in case of invalid input
        while (getchar() != '\n');
    }

    // Free previous allocation if exists
    freePCBArray();

    // Allocate and initialize the PCB array
    initializePCBArray(maxProcessesNumber);

    // Initialize the root process with no children if processes are defined
    if (numberOfProcesses > 0) {
        *pcb[0].parent = 0; // Or set to -1 if it's prefered indicating no parent
        *pcb[0].firstChild = -1;
        *pcb[0].olderSibling = -1;
        *pcb[0].youngerSibling = -1;
    }

    printPCBTable();
}

void option2(void) {
    int p, q = 0;
    
    // Find an unused PCB entry for the new child
    while (q < numberOfProcesses && *pcb[q].parent != -1) q++;
    if (q == numberOfProcesses) {
        printf("Error: No available PCB entries for a new process.\n\n");
        return;
    }
    
    printf("Enter the parent process index: ");
    scanf("%d", &p);

    // Check for the existence of the parent process
    // Assuming index 0 can be a valid parent
    if (p < 0 || p >= numberOfProcesses || (*pcb[p].parent == -1 && p != 0)) {
        printf("Error: No existing process for the given parent index.\n\n");
        return;
    }

    *pcb[q].parent = p;
    *pcb[q].firstChild = -1;
    *pcb[q].olderSibling = *pcb[p].firstChild != -1 ? *pcb[p].firstChild : -1;
    *pcb[q].youngerSibling = -1;

    if (*pcb[p].firstChild == -1) {
        *pcb[p].firstChild = q;
    } else {
        int currentChild = *pcb[p].firstChild;
        while (*pcb[currentChild].youngerSibling != -1) {
            currentChild = *pcb[currentChild].youngerSibling;
        }
        *pcb[currentChild].youngerSibling = q;
        *pcb[q].olderSibling = currentChild;
    }

    printPCBTable();
}

void recursivelyDestroyDescendants(int processIndex) {
    // Base case: invalid index
    if (processIndex == -1) return;

    // Recursively destroy all descendants of the first child
    if (pcb[processIndex].firstChild != NULL && *pcb[processIndex].firstChild != -1)
        recursivelyDestroyDescendants(*pcb[processIndex].firstChild);

    // Move to destroy siblings only after the current process and its descendants are handled
    if (pcb[processIndex].youngerSibling != NULL && *pcb[processIndex].youngerSibling != -1)
        recursivelyDestroyDescendants(*pcb[processIndex].youngerSibling);

    // 'Destroy' this process by resetting its PCB entries
    *pcb[processIndex].parent = -1;
    *pcb[processIndex].firstChild = -1;
    *pcb[processIndex].olderSibling = -1;
    *pcb[processIndex].youngerSibling = -1;
}

void option3(void) {
    int processIndex;
    printf("Enter the process index whose descendants are to be destroyed: ");
    scanf("%d", &processIndex);

    // Validate the process index
    if (processIndex < 0 || processIndex >= numberOfProcesses || pcb[processIndex].parent == NULL || *pcb[processIndex].parent == -1) {
        printf("Invalid process index.\n");
        return;
    }

    // Start the recursive destruction from the first child of the specified process
    if (pcb[processIndex].firstChild != NULL && *pcb[processIndex].firstChild != -1) {
        recursivelyDestroyDescendants(*pcb[processIndex].firstChild);
        // Reset the firstChild pointer after all descendants are destroyed
        *pcb[processIndex].firstChild = -1;
    }

    printPCBTable();
}

void option4(void) {
    freePCBArray();
    printf("Memory freed. Exiting program.\n");
}

int main(void) {
    int option = 0;

    while (option != 4) {
        printf(MENU_OF_OPTIONS);
        scanf("%d", &option);

        switch (option) {
            case 1: option1(); break;
            case 2: option2(); break;
            case 3: option3(); break;
            case 4: option4(); break;
            default: printf("Error: Invalid selection. Please enter a number between 1 and 4.\n\n"); break;
        }
    }

    return 0;
}
