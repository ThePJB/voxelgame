float slow_start2(float x) {return x*x;}
float slow_start3(float x) {return x*x*x;}
float slow_start4(float x) {return x*x*x*x;}

float slow_stop2(float x) {return 1 - (slow_start2(1-x));}
float slow_stop3(float x) {return 1 - (slow_start3(1-x));}
float slow_stop4(float x) {return 1 - (slow_start4(1-x));}

