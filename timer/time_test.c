#include <stdio.h>
#include <time.h>

int main ()
{
	time_t start_time, end_time;
	struct tm * time_info;
  char buffer [6];
  double diff;

  time ( &start_time );
  usleep(4000000);
  time ( &end_time );
  
  diff = difftime(end_time, start_time);
  printf("diff: %f\n",diff);
  printf("start_time sec: %d - %d\n",start_time,end_time);

  time_info = localtime ( &start_time );
  strftime (buffer,6,"%M.%S",time_info);
  puts (buffer);
    
  time_info = localtime ( &end_time );
  strftime (buffer,6,"%M.%S",time_info);
  puts (buffer);

  end_time -= start_time;
  printf("end_time -= start_time: %d\n",end_time);
  
  return 0;
}
