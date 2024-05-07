#include "cliente.h"



int Envia(HANDLE hPipe) {
    DWORD cbBytesWrite = 0;
    BOOL fSuccess = FALSE;
    OVERLAPPED OvW = { 0 };
    HANDLE WriteReady;
    WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
    ZeroMemory(&OvW, sizeof(OvW));
    ResetEvent(WriteReady);
    OvW.hEvent = WriteReady;
    ZeroMemory(&OvW, sizeof(OvW));
    ResetEvent(WriteReady);
    OvW.hEvent = WriteReady;
    fSuccess = WriteFile(hPipe, &utilizador, Msg_Sz, &cbBytesWrite, &OvW);
    if (!fSuccess) {
        _tprintf(TEXT("[ERRO] Falha ao enviar a mensagem! (WriteFile)\n"));
        return -1;
    }
    WaitForSingleObject(WriteReady, INFINITE);
    _tprintf(TEXT("[CLIENTE] Mensagem enviada ao servidor!\n"));
    return 0;
}

int Recebe(HANDLE hPipe) {
    DWORD cbBytesRead = 0;
    BOOL fSuccess = FALSE;
    OVERLAPPED OvR = { 0 };
    HANDLE ReadReady;
    ReadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
    ZeroMemory(&OvR, sizeof(OvR));
    ResetEvent(ReadReady);
    OvR.hEvent = ReadReady;

    fSuccess = ReadFile(hPipe, &utilizador, Msg_Sz, &cbBytesRead, &OvR);
    if (!fSuccess && GetLastError() != ERROR_IO_PENDING) {
        _tprintf(TEXT("[ERRO] Falha ao receber a mensagem! (ReadFile)\n"));
        return -1;
    }
    // Verifica se a opera  o est  pendente (n o h  dados imediatamente dispon veis)
    if (!fSuccess) {
        // A opera  o est  pendente, aguardar at  que ela seja conclu da
        WaitForSingleObject(ReadReady, INFINITE);
        // Verifica se a opera  o foi conclu da com sucesso
        fSuccess = GetOverlappedResult(hPipe, &OvR, &cbBytesRead, FALSE);
        if (!fSuccess) {
            _tprintf(TEXT("[ERRO] Falha na leitura do pipe! (GetOverlappedResult)\n"));
            return -1;
        }
    }
    _tprintf(TEXT("[CLIENTE] Mensagem recebida do servidor!\n"));
    return 0;
}


int _tmain(int argc, LPTSTR argv[]) {
    HANDLE hPipe;
    DWORD cbBytesRead = 0, cbBytesWrite = 0;
    BOOL fSuccess = FALSE, resposta = FALSE;
    OVERLAPPED OvR = { 0 };
    OVERLAPPED OvW = { 0 };
    TCHAR context[MAX_TAM];

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    utilizador.login = FALSE;

    hPipe = CreateFile(PIPE_NAME_CLIENTS, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("[ERRO] Falha ao conectar-se ao servidor! (CreateFile)\n"));
        return -1;
    }
    if (!WaitNamedPipe(PIPE_NAME_CLIENTS, 30000)) {
        _tprintf(TEXT("[ERRO] Falha ao conectar-se ao servidor! (WaitNamedPipe)\n"));
        return -1;
    }
    _tprintf(TEXT("[CLIENTE] Conectado ao servidor!\n Inicio... \n"));

    
    TCHAR comando[MAX_TAM];
    TCHAR* argumentos[MAX_TAM];
    int nArgs;
    do {
        nArgs = 0;
        if (utilizador.login == FALSE) {
            _tprintf(TEXT("\nlogin <username> <password> para entrar\n"));
            _fgetts(comando, MAX_TAM, stdin);
            comando[_tcslen(comando) - 1] = '\0'; // Remover o caractere de nova linha
            argumentos[0] = _tcstok_s(comando, TEXT(" "), &context);
            while (argumentos[nArgs] != NULL) {
                argumentos[++nArgs] = _tcstok_s(NULL, TEXT(" "), &context);
            }
            nArgs -= 1;

            if (_tcscmp(argumentos[0], _T("login")) == 0 && nArgs == 2) {
                wcscpy_s(utilizador.username, _countof(utilizador.username), argumentos[1]);
                wcscpy_s(utilizador.password, _countof(utilizador.password), argumentos[2]);
                
                
                Envia(hPipe);
                Sleep(1000);
                Recebe(hPipe);
                
                /*HANDLE ReadReady, WriteReady;
                WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
                ReadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
                // Configurar opera  o overlapped para escrita
                ZeroMemory(&OvW, sizeof(OvW));
                ResetEvent(WriteReady);
                OvW.hEvent = WriteReady;
                fSuccess = WriteFile(hPipe, &utilizador, Msg_Sz, &cbBytesWrite, &OvW);
                if (!fSuccess) {
                    _tprintf(TEXT("[ERRO] Falha ao enviar a mensagem! (WriteFile)\n"));
                    return -1;
                }
                WaitForSingleObject(WriteReady, INFINITE);
                _tprintf(TEXT("[CLIENTE] Mensagem enviada ao servidor!\n"));
                // Configurar opera  o overlapped para leitura
                ZeroMemory(&OvR, sizeof(OvR));
                ResetEvent(ReadReady);
                OvR.hEvent = ReadReady;
                fSuccess = ReadFile(hPipe, &utilizador, Msg_Sz, &cbBytesRead, &OvR);
                if (!fSuccess && GetLastError() != ERROR_IO_PENDING) {
                    _tprintf(TEXT("[ERRO] Falha ao receber a mensagem! (ReadFile)\n"));
                    return -1;
                }
                // Verifica se a opera  o est  pendente (n o h  dados imediatamente dispon veis)
                if (!fSuccess) {
                    // A opera  o est  pendente, aguardar at  que ela seja conclu da
                    WaitForSingleObject(ReadReady, INFINITE);
                    // Verifica se a opera  o foi conclu da com sucesso
                    fSuccess = GetOverlappedResult(hPipe, &OvR, &cbBytesRead, FALSE);
                    if (!fSuccess) {
                        _tprintf(TEXT("[ERRO] Falha na leitura do pipe! (GetOverlappedResult)\n"));
                        return -1;
                    }
                }*/
                if (utilizador.login == TRUE) {
                    _tprintf(TEXT("Login efetuado com sucesso!\nBem vindo: %s\t Saldo:%d"), utilizador.username,utilizador.saldo);
                }
                else {
                    _tprintf(TEXT("Invalid username or password\n"));
                }
            }
            else if (_tcsicmp(argumentos[0], TEXT("exit")) != 0) {
                _tprintf(TEXT("Comando invalido!!!!!!!!!!!!!\n"));
            }
        }
        else {
            _tprintf(TEXT("\nComando: "));
            _fgetts(comando, MAX_TAM, stdin);
            comando[_tcslen(comando) - 1] = '\0'; // Remover o caractere de nova linha
            argumentos[0] = _tcstok_s(comando, TEXT(" "), &context);
            while (argumentos[nArgs] != NULL) {
                argumentos[++nArgs] = _tcstok_s(NULL, TEXT(" "), &context);
            }
            nArgs -= 1;
            if (_tcsicmp(argumentos[0], TEXT("listc")) == 0 && nArgs == 0) {
 

                _tprintf(TEXT("listc\n"));
            }
            else if (_tcsicmp(argumentos[0], TEXT("buy")) == 0 && nArgs == 2) {


                _tprintf(TEXT("buy\n"));
            }
            else if (_tcsicmp(argumentos[0], TEXT("sell")) == 0 && nArgs == 2) {

                //Envia(hPipe, WriteReady, ReadReady);
                _tprintf(TEXT("sell\n"));
            }
            else if (_tcsicmp(argumentos[0], TEXT("balance")) == 0 && nArgs == 0) {

                //Envia(hPipe, WriteReady, ReadReady);
                _tprintf(TEXT("balance\n"));
            }
            else if (_tcsicmp(argumentos[0], TEXT("exit")) != 0) {
                _tprintf(TEXT("Comando inv lido\n"));
            }
        }
    } while (_tcsicmp(argumentos[0], TEXT("exit")) != 0);
    _tprintf(TEXT("A sair..."));
    CloseHandle(hPipe);
    return 0;
}