��������� ftp ����������
AT+FTPSERV=�ftp.site.ru� � ����� ftp �������
AT+FTPUN=�login� � �����
AT+FTPPW=�password� � ������
AT+FTPPUTPATH="/" � ����
AT+FTPPUTNAME=�test.txt� � ��� �����
AT+FTPMODE=1 � ����� ������ ftp
1, � ���������. ����� �� ���������
0, � ��������.
AT+FTPTYPE=�A� � ��� ������������ ������.
A, � ������ ASCII �������.
I, � �������� �����.
AT+FTPPUTOPT=�APPE� � ����� �������� �����.
�APPE� � �������� � ����� �����
�STOR� � ������� ����. ���� ���� ����������, �� �� ����� �����������.
�STOU� � ������� ���������� ����. ���� ���� ����������, �� ����� ������ ���� � �������� ������ � ����������� � ����� �������.
AT+FTPPUT=1 � �������� ftp ����������
+FTPPUT:1,1,1260 � ������ �� ������ �������. ������������ ����� ������ 1260 ����.
AT+FTPPUT=2,10 � ������ 10 ��������.
+FTPPUT:2,10 � ����� ��������� ����������� ����� ������ 10 ��������. ��� �� �����.
AT+FTPPUT:2,0 � �������� ftp ����������.

��� ������ ��� ����������� ����������, ������ PUT �������� �� GET. ��� AT+FTPPUTPATH ���������� AT+FTPGETPATH.
AT+FTPGET=1 � �������� ftp ����������.
+FTPGET:1,1 � ������ ��� ������ ��������.
AT+FTPGET=2,10 � ������ 10 ���� ������. ���� ������ ������, �� �������� ������� ����.
+FTPGET:2,10 � ������� ������ ����� ��� ����������.
��������� ������, ���� ��������� ����� �� ����� ����� ������ +FTPGET:2,0.
AT+FTPGET:2,0 � �������� ftp ����������.


 TO-DO
    
	 1. 