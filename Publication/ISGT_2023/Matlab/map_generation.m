load('resultsEV_HP_ExcursionsEVReFLEX40_100.mat')
A=mean(Number_Excursions,3);
surf(A/4)