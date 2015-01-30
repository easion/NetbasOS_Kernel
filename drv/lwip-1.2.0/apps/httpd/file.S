	bits	32
	SECTION .data

%macro	EXPORT	1
	GLOBAL %1
	%1:
	GLOBAL _%1
	_%1:
%endmacro

%macro	IMPORT 1
%ifdef UNDERBARS
	EXTERN _%1		; GCC for DOS (DJGPP; COFF)
	%define %1 _%1
%else
	EXTERN %1		; GCC for Linux (ELF)
%endif
%endmacro

EXPORT data_http_documentation_html
	incbin 'fs/http/documentation.html'
data_http_documentation_html_end:

EXPORT data_http_documentation_html_len 
	dd data_http_documentation_html_end - data_http_documentation_html


EXPORT file_http_index_html
	incbin 'fs/http/index.html'
file_http_index_html_end:

EXPORT file_http_index_html_len 
	dd data_http_documentation_html_end - file_http_index_html


