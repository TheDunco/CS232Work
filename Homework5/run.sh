gcc vacc_clinic.c -o vacc_clinic -lpthread && rm latestOutput.txt && ./vacc_clinic 2>&1 | tee latestOutput.txt
