#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_APPLIANCES 100

typedef struct {
  char name[50];
  float power_rating; // in watts
  float total_cost;
} Appliance;

Appliance appliances[MAX_APPLIANCES];
int appliance_count = 0;

void appliance_details() {
  printf("\n");
  printf("Enter the number of appliances: ");
  scanf("%d", &appliance_count);
  printf("\n");
  printf("NOTE: Enter the acronyms for 'appliances name' (Ex:  WM for Washing "
         "Machine)\n");
  printf("\n");
  for (int i = 0; i < appliance_count; i++) {
    printf("Enter the name of appliance %d (acronym): ", i + 1);
    scanf("%s", appliances[i].name);
    printf("Enter the power rating (in watts) of %s: ", appliances[i].name);
    scanf("%f", &appliances[i].power_rating);
    printf("\n");
  }
  printf("\n");
  printf("Appliance details entered successfully.\n");
  printf("\n");
}

void log_usage() {
  printf("\n");
  printf("NOTE: If there are many units of same appliance, enter the sum of "
         "duration used.\n");
  printf("\n");
  for (int i = 0; i < appliance_count; i++) {
    float duration;
    printf("Enter the duration (in hours) for %s: ", appliances[i].name);
    scanf("%f", &duration);

    float units_consumed = (appliances[i].power_rating / 1000) *
                           duration; // converting watts to kilowatts
    float cost = units_consumed * 6;
    appliances[i].total_cost += cost;
    printf("%s consumed %.2f kWh costing ₹%.2f\n", appliances[i].name,
           units_consumed, cost);
  }
}

void amount() {
  printf("\n");
  float daily_cost = 0;
  for (int i = 0; i < appliance_count; i++) {
    daily_cost += appliances[i].total_cost;
  }
  printf("Total cost for the day : ₹ %.2f\n", daily_cost);
  printf("\n");
  printf("Number of free units per month as per Gruha Jyoti Sheme = 200\n");
  int month;
  printf("Enter the month (01 to 12)\t ");
  scanf("%d", &month);
  int d;
  if (month % 2 != 0 && month <= 7) {
    d = 31;
  } else if (month % 2 == 0 && month > 7) {
    d = 31;
  } else if (month == 02) {
    d = 28;
  } else if (month != 02 && month % 2 == 0 && month < 7) {
    d = 30;
  } else if (month % 2 != 0 && month > 7) {
    d = 30;
  }
  float free_units = 200.00 / d;
  printf("Number of free units per day during this month = 200/%d = %.2f\n", d,
         free_units);
  float sa = 6 * free_units;
  printf("Subsidy amount per day during this month =₹ %.2f\n", sa);
  printf("\n");
  float ul, am, um;
  if (daily_cost < sa) {
    ul = (sa - daily_cost) / 6;
    printf("Congratulations, you've consumed %.2f units less than the subsidy "
           "limit\n",
           ul);
    printf("Keep it up\n");
    printf("Amount to be paid for the day = ₹ 0.0 rupees\n");
  } else if (daily_cost == sa) {
    printf("Wow, you've exactly met the subsidy limit\n");
    printf("Keep it up\n");
    printf("Amount to be paid for the day = ₹ 0.0\n");
  } else {
    am = daily_cost - sa;
    um = am / 6;
    printf("Oops, you've consumed %.2f units more than the subsidy limit\n",
           um);
    printf("Amount to be paid for the day after deduction of subsidy amount = "
           "₹ %.2f\n",
           am);
  }
}

int main() {
  int choice;
  printf("===Daily Electricity Bill Calculator===\n");

  do {
    printf("\n");
    printf("Choice no.\t Action\n");
    printf("   1.        Enter appliance details\n");
    printf("   2.        Log Usage\n");
    printf("   3.        View today's bill\n");
    printf("\n");
    printf("Enter the choice no. for the action to be performed: ");
    scanf("%d", &choice);

    switch (choice) {
    case 1:
      printf("\n");
      printf("==APPLIANCE DETAILS==\n");
      appliance_details();
      break;
    case 2:
      printf("\n");
      printf("==LOG USAGE==\n");
      log_usage();
      break;
    case 3:
      printf("\n");
      printf("==TODAY'S BILL==\n");
      amount();
      printf("Exiting...\n");
      break;
    default:
      printf("Invalid choice. Please try again.\n");
    }
  } while (choice != 3);

  return 0;
}
