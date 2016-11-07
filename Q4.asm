; Constantes
STDIN equ 0
STDOUT equ 1
SYS_READ equ 3
SYS_WRITE equ 4
SYS_EXIT equ 1

; Inicia valores das strings na memória
section .data
  msg_prompt_base: db 'Digite o tamanho da base: '
  msg_prompt_base_size: equ $-msg_prompt_base
  msg_prompt_height: db 'Digite o tamanho da altura: '
  msg_prompt_height_size: equ $-msg_prompt_height
  msg_star: db '*'
  msg_space: db ' '
  msg_new_line: db 10 ; Código ASCII referente a \n

; Reserva 1 byte na memória para os inputs
section .bss
  base: resb 1
  height: resb 1

section .text
global _start

_start:
  ; Imprime o prompt da base na tela
  mov eax, SYS_WRITE
  mov ebx, STDOUT
  mov ecx, msg_prompt_base
  mov edx, msg_prompt_base_size
  int 80h

  ; Lê o valor da base e insere na memória
  mov eax, SYS_READ
  mov ebx, STDIN
  mov ecx, base
  mov edx, 4
  int 80h

  sub dword[base], 0A30h ; Subtrai o valor pego do input por 2680 (0xA30 em hexadecimal) para pegar o char digitado
  push dword[base] ; Coloca na pilha de memória

  ; Imprime o prompt da altura na tela
  mov eax, SYS_WRITE
  mov ebx, STDOUT
  mov ecx, msg_prompt_height
  mov edx, msg_prompt_height_size
  int 80h

  ; Lê o valor da altura e insere na memória
  mov eax, SYS_READ
  mov ebx, STDIN
  mov ecx, height
  mov edx, 4
  int 80h

  sub dword[height], 0A30h ; Subtrai o valor pego do input por 2680 (0xA30 em hexadecimal) para pegar o char digitado
  push dword[height] ; Coloca na pilha de memória

  call PRINT_NEW_LINE

  ; Recupera os valores da altura e da base, respectivamente
  pop ebx
  pop eax

  ; Caso algum dos valores sejam 0, encerra a execução
  cmp eax, 0
  jmp EXIT
  cmp ebx, 0
  jmp EXIT

  mov ecx, eax ; Copia o valor da base para o registrador ecx

  call PRINT_FULL_STAR ; Imprime a primeira linha de estrelas

  ; Caso a altura seja 2, pula o código para impressão da última linha
  cmp ebx, 2
  je END_FOR

  ; Caso a altura seja 1, encerra a execução
  cmp ebx, 1
  je EXIT

  sub ebx, 2 ; Diminui 2 da altura referente à primeira e última linha

  ; Imprime ALTURA - 2 linhas de estrelas nas bordas e espaços vazios
FOR:
  ; Quando o registrador for igual a 0, "encerra a execução do FOR"
  cmp ebx, 0
  je END_FOR

  dec ebx ; Decrementa o valor do registrador

  call PRINT_BORDER_STAR ; Imprime uma linha de estrelas apenas ns bordas e espaços vazios

  jmp FOR ; Volta para o começo do "FOR"
END_FOR:

  call PRINT_FULL_STAR ; Imprime a última linha de estrelas
  jmp EXIT ; Encerra execução

  ; Sub-rotina para imprimir uma linha com apenas estrelas
PRINT_FULL_STAR:
FULL_FOR:
  cmp eax, 0
  je FULL_END_FOR

  dec eax

  call PRINT_STAR
  jmp FULL_FOR
FULL_END_FOR:
  call PRINT_NEW_LINE
  mov eax, ecx
  ret

  ; Sub-rotina para imprimir uma linha com duas estrelas nas bordas e espaços vazios no meio
PRINT_BORDER_STAR:
  BORDER_FOR:
  ; Caso seja a primeira ou última passada, imprime uma estrela
  cmp eax, ecx
  je BORDER_STAR
  cmp eax, 1
  je BORDER_STAR

  call PRINT_SPACE ; Caso não seja, imprime espaço

  dec eax
  jmp BORDER_FOR
BORDER_STAR:
  call PRINT_STAR

  ; Caso seja a ultima passada, "encerra o for"
  cmp eax, 1
  je BORDER_END_FOR

  dec eax

  jmp BORDER_FOR
BORDER_END_FOR:

  call PRINT_NEW_LINE
  mov eax, ecx
  ret

  ; Sub-rotina para imprimir um espaço vazio (Salva os valores dos registradores na memória e depois da execução coloca seus valores nos registradores)
PRINT_SPACE:
  push eax
  push ebx
  push ecx
  push edx
  mov eax, SYS_WRITE
  mov ebx, STDOUT
  mov ecx, msg_space
  mov edx, 1
  int 80h
  pop edx
  pop ecx
  pop ebx
  pop eax
  ret

  ; Sub-rotina para imprimir uma estrela (Salva os valores dos registradores na memória e depois da execução coloca seus valores nos registradores)
PRINT_STAR:
  push eax
  push ebx
  push ecx
  push edx
  mov eax, SYS_WRITE
  mov ebx, STDOUT
  mov ecx, msg_star
  mov edx, 1
  int 80h
  pop edx
  pop ecx
  pop ebx
  pop eax
  ret

  ; Sub-rotina para quebrar uma linha (Salva os valores dos registradores na memória e depois da execução coloca seus valores nos registradores)
PRINT_NEW_LINE:
  push eax
  push ebx
  push ecx
  push edx
  mov eax, SYS_WRITE
  mov ebx, STDOUT
  mov ecx, msg_new_line
  mov edx, 1
  int 80h
  pop edx
  pop ecx
  pop ebx
  pop eax
  ret

  ; Gracefull exit
EXIT:
  mov eax, SYS_EXIT
  mov ebx, STDIN
  int 80h
