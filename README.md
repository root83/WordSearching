# 문서 색인 / 검색기능 개발

## 개발 환경
>>OS : X86-64 redhat lunux
>>개발 언어 : C언어
>>사용 라이브러리 : stdlib, stdio, string
>>컴파일러 : gcc 4.1.2

##요구사항
>>1. 문서번호와 내용으로 구성된 여러 개의 문서를 입력 받아 색인을 구축한다.
>>2. 검색어를 입력받아서 검색된 문서 번호를 리턴한다.
>>3. 리턴하는 문서번호의 순서는 임의대로 구현한다.
>>4. 색인 시 문서 내용과 검색어의 형태소 분석은 공백을 기준으로 한다.
>>5. 색인의 저장은 메모리에 한다.
>>6. 역파일에서 형태소의 검색은 Sequential search 이외의 임의의 검색 로직을 적용한다.
>>7. 검색어 형태소 간의 관계는 AND와 OR를 옵션으로 제공한다.

##실행 환경
>>리눅스 OS

##파일 설명
>>structure.h : 자료 구조 헤더
>>btree.c btree.h : B-tree 삽입, 검색, 삭제 기능 등 관련
>>wordsearch.c wordsearch.h : 문서 역파일, 검색어 입력 및 검색, 결과 출력 등, main 함수
>>wordsearh : 실행 바이너리 파일
>>Makefile : 메이크파일

##제약 조건
>>문서는 숫자와 영어 알파벳으로만 구성되어 있어야 한다.
>>검색어 입력 시 단어 단위로만 검색이 이루어 진다.
>>Byte 단위의 특수문자를 제외하므로 2Byte이상의 문자는 색인/검색이 정상적으로 처리되지 않는다.
	

##실행 절차
>>1. wordsearch 바이너리 파일 실행
>>2. 역파일 할 문서 파일 명 입력
>>3. 's!' 입력을 통해 검색 모드로 변경
>>4. 검색어 입력
>>>>1) 문장 입력 시 공백으로 형태소 구분
>>>>2) 형태소에 쌍따옴표(")가 전 혹은 후로 붙어 있을 경우 AND 조건으로 검색
>>5. 'q!' 입력을 통해 종료


##역파일(색인 기능)
>>1. 입력된 문서를 공백 기준으로 키워드를 추출, 키워드, 해당 문서 번호, 문서에서 검색된 횟수를 저장한다.
>>>>(초기 개발 시 검색 결과 랭킹에 사용하기 위하여 검색 횟수를 수집/저장은 하였지만 사용되지 않음)
>>2. 특수문자는 Byte 단위로 제거한다
>>>>(예 : don't -> dont로 대체)
>>3. 각각의 결과는 링크드 리스트로 저정한다.
>>>>(keyword set과 document set)
>>4. 추출된 keyword set을 B-tree 구조로 저장한다.
>>5. Node 하나 당 최대 11개의 Keyword set을 저장한다.
>>6. B-tree 구조에서 Key값이 저장되는 위치는 key값 간의 strcmp 결과를 이용하여 정한다.
>>7. 이미 B-tree에 존재하는 Key 일 경우 문서 색인을 확인하여 새로운 문서일 경우 문서 번호를 추가한다.


##검색 기능
>>1. 검색어 입력 시 공백단위로 구분하여 각각의 형태소를 링크드 리스트로 저장한다.
>>2. 쌍따옴표로 입력된 검색어는 AND 조건, 이외에는 OR 조건 검색어로 인식한다.
>>3. 검색어가 포함하는 문서일 경우 빈도를 증가한다.
>>4. 검색 결과는 가장 많은 빈도를 가진 (가장 많은 종류의 검색어를 포함하는) 문서 순으로 출력한다.
>>5. 검색 시 B-tree head로 부터 Key 값간의 strcmp 결과를 이용하여 검색어가 존재하는 리프 node를 찾아나간다.
>>6. OR 조건을 먼저 검색하고 해당 결과 중 AND 조건 검색 결과에 포함되지 않는 결과는 제외한다.


##기타
>>현재 업무량이 과다하여 야간을 이용해 코딩하다보니 가독성이 떨어집니다.
>>마지막 검색어 입력 및 키워드 추출, 검색 결과 비교 및 정렬하는 기능을 하나의 함수에 몰아 넣었습니다.
>>과제를 수행하면서 미약하나마 검색기가 동작하는 원리 등을 경험할 수 있어서 뿌듯합니다.



