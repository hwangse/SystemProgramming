import requests
from bs4 import BeautifulSoup

urlList=list()	#방문 주소를 저장하는 문자열 list
startLink='http://cspro.sogang.ac.kr/~gr120170213/'

#link 들을 탐색하는 재귀함수 
#a tag를 통하여 새로운 link를 얻을 때 마다 호출된다. 
def traverse_link(soup,url) : 
	global startLink

	results=soup.find_all('a')

	for i in results : 
		tmp=i.get('href')										#new link get

		if tmp != None : 
			if len(tmp)==0 or (tmp[0]=='?' or tmp[0]=='#'): 	#invalid link
				continue
			else : 
				if len(tmp)>7 and tmp[:7]=='http://'  : 
					newLink=tmp
				else : 
					newLink=startLink+tmp

				try : 
					r=requests.get(newLink)
					if(r.status_code == 200) :  				#valid link 일때만 재귀 호출
						flag = 0 
						if len(urlList) >0 : 	 				#중복된 link check
							for i in range(len(urlList)) : 
								if urlList[i] == newLink : 		#중복되는 link 라면
									flag = 1     				#invalid link
						if(flag==0) :			 				#여기까지 왔다면 valid link이다.
							urlList.append(newLink)
							count=len(urlList)
	
							if(count==1) : 
								url.write(newLink)
							else : 
								url.write('\n'+newLink)
							soup=BeautifulSoup(r.content,"html.parser")

							L=list(soup.children)
							filename='Output_'+str(count).zfill(4)+'.txt'

							newfp=open(filename,"w")			#create output.txt file pointer 
							newfp.write(soup.get_text())
							newfp.close()						#delete output.txt file pointer
							traverse_link(soup,url)
					else : 
						pass									#invalid link
																#404,403,401일때 
				except requests.exceptions.ConnectionError : 
					print("connection error occurred")
				except requests.exceptions.RequestException : 
					print("request exception occurred")

def print_error_code(code) : 
	if code == 403 : 
		print("403 : 금지됨")
	elif code == 404 : 
		print("404 : 찾을 수 없음")
	elif code == 401 : 
		print("401 : 권한 없음 " )

"""---------------------- [MAIN FUNCTION START] --------------------"""
try : 
	r=requests.get(startLink)

	 #이 부분 더 자세히로 변경하기
	if r.status_code != 200 : 
		print_error_code(r.status_code)
	else : 
		soup=BeautifulSoup(r.content,"html.parser")
		url=open("URL.txt","w")									#create URL.txt file pointer 
		traverse_link(soup,url)									#link 탐색 시작 
		url.close()												#delete URL.txt file pointer

#exception handling 
except requests.exceptions.ConnectionError : 
	print("connection error occurred")
except requests.exceptions.RequestException : 
	print("request exception occurred")
except requests.exceptions.HTTPError : 
	print("HTTP error occurred")
except requests.exceptions.SSLError : 
	print("SSL error occurred")
except requests.exceptions.InvalidURL : 
	print("The URL provided was somehow invalid")

"""---------------------- [MAIN FUNCTION END] ------------------------"""
