#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

const int HANDLE_COUNT = 3;

size_t save_sina_page(void *buffer, size_t size, size_t count, void *user_p)
{
	return fwrite(buffer, size, count, (FILE *)user_p);
}

int main(void){

		CURL* handles[HANDLE_COUNT];
		CURLM* multi_handle;

		int still_running;

		for(int i=0;i<HANDLE_COUNT;i++){
				handles[i] = curl_easy_init();
		}

		FILE *fp_sina0 = fopen("sina0.html", "ab+");
		FILE *fp_sina1 = fopen("sina1.html", "ab+");
		FILE *fp_sina2 = fopen("sina2.html","ab+");

		curl_easy_setopt(handles[0],CURLOPT_URL,"www.sina.com");
		curl_easy_setopt(handles[0], CURLOPT_WRITEFUNCTION, &save_sina_page);
		curl_easy_setopt(handles[0], CURLOPT_WRITEDATA, fp_sina0);

		curl_easy_setopt(handles[1],CURLOPT_URL,"www.sina.com");
		curl_easy_setopt(handles[1], CURLOPT_WRITEFUNCTION, &save_sina_page);
		curl_easy_setopt(handles[1], CURLOPT_WRITEDATA, fp_sina1);

		curl_easy_setopt(handles[2],CURLOPT_URL,"www.sina.com");
		curl_easy_setopt(handles[2], CURLOPT_WRITEFUNCTION, &save_sina_page);
		curl_easy_setopt(handles[2], CURLOPT_WRITEDATA, fp_sina2);

		multi_handle = curl_multi_init();

		for(int i=0;i<HANDLE_COUNT;i++){
				curl_multi_add_handle(multi_handle,handles[i]);
		}

		curl_multi_perform(multi_handle,&still_running);
		cout<<still_running<<endl;

		while(still_running){
				struct timeval timeout;
				int rc;
				fd_set fdread;
				fd_set fdwrite;
				fd_set fdexcep;
				int maxfd = -1;
				long curl_timeo = -1;

				FD_ZERO(&fdread);
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdexcep);

				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
				if((CURLM_OK ==curl_multi_timeout(multi_handle,&curl_timeo)) 
						&& (curl_timeo >=0 )){
						timeout.tv_sec = curl_timeo / 1000;
						timeout.tv_sec ++;
				}

				curl_multi_fdset(multi_handle,&fdread,&fdwrite,&fdexcep,&maxfd);

				rc = select(maxfd+1,&fdread,&fdwrite,&fdexcep,&timeout);

				if(rc >= 0){
					curl_multi_perform(multi_handle,&still_running);
				}else{
						cout<<"select error"<<endl;
				}
				cout<<still_running<<endl;
		}

		//===========================================================
		// get result information

		CURLMsg* msg;
		int msg_in_queue = -1;
		while(msg = curl_multi_info_read(multi_handle,&msg_in_queue)){
				if(msg->msg == CURLMSG_DONE){
						cout<<"sina done"<<endl;
				}
		}

		curl_multi_cleanup(multi_handle);
		for(int i=0;i<HANDLE_COUNT;i++){
				curl_easy_cleanup(handles[i]);
		}

		return 0;
}
