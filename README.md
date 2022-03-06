# webgrab_v2

## Description:
It's multi thread app with 3 threads. First thread listen user commands. <br /> 
Second and third thread check and process items from the queue.
<br /> 
The queue is protected from access by multiple thread at the same time.

## Main functionality:
The application downloads a file from the specified URLs. Only text files are currently supported and tested.


## Examples
Steps:<br/>
1. Start service (./webgrab)
2. ./webgrab -d url_adress1
3. ./webgrab -d url_adress2 (url_adress1 is still downloading)
4. ./webgrab -d url_adress3 (url_adress1 and url_adress2 is still downloading)) -> waiting for the release of the thread and then it starts downloading

<br/>

1. Start service (./webgrab)
2. ./webgrab -d url_adress1
3. ./webgrab -s (url_adress1 is still downloading) -> service will shut down after download url_adress1
4. ./webgrab -d url_adress2 ( message: connection failed ) -> thread for listening is exit, now work only the threads for downloading

## How to run:

****./webgrab**** ... the program starts and listens commands<br/>
****./webgrab -d URL adress**** ... the program is started and send URL adress to service ( URL is added to the queue)<br/>
****./webgrab -s**** ... the main service is switched off (it will turn off completely after the ongoing download is completed)


## File name for downloaded files
random_number range 100 - 999<br/>
output_{random_number}.out <br/>

###### Examples of URL addresses
* https://perishablepress.com/robots.txt;
* https://www.wordfrequency.info/samples/lemmas_60k.txt
* https://www.wordfrequency.info/samples/words_219k.txt
* https://www.wordfrequency.info/samples/lemmas_60k_words.txt
* https://ars.els-cdn.com/content/image/1-s2.0-S0960982203005347-mmc6.txt
* https://norvig.com/ngrams/count_2w.txt
* https://norvig.com/big.txt
