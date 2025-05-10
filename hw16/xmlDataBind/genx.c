#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xml/details/genx/genx.h"
#include "xml/details/genx/charProps.h"

/* Simple implementation of genx API stubs */
genxWriter genxNew(void) {
    return malloc(1); /* Just a dummy non-NULL pointer */
}

void genxDispose(genxWriter w) {
    free(w);
}

genxStatus genxStartDocSender(genxWriter w, genxSender sender, void *userData) {
    return GENX_SUCCESS;
}

const char *genxGetErrorMessage(genxWriter w, genxStatus status) {
    return "Genx stub implementation";
}

genxStatus genxXmlDeclaration(genxWriter w, constUtf8 version, constUtf8 encoding, constUtf8 standalone) {
    return GENX_SUCCESS;
}

genxStatus genxEndDocument(genxWriter w) {
    return GENX_SUCCESS;
}

genxStatus genxStartElementLiteral(genxWriter w, constUtf8 xmlns, constUtf8 type) {
    return GENX_SUCCESS;
}

genxStatus genxEndElement(genxWriter w) {
    return GENX_SUCCESS;
}

genxStatus genxAddNamespaceLiteral(genxWriter w, constUtf8 uri, constUtf8 prefix, genxNamespace *ns) {
    static char dummy = 0;
    if (ns) *ns = &dummy;
    return GENX_SUCCESS;
}

genxStatus genxUnsetDefaultNamespace(genxWriter w) {
    return GENX_SUCCESS;
}

genxStatus genxGetNamespacePrefix(genxNamespace ns, constUtf8 *prefix) {
    return GENX_SUCCESS;
}

genxStatus genxDeclareNamespace(genxWriter w, constUtf8 uri, constUtf8 prefix, genxNamespace *ns) {
    static char dummy = 0;
    if (ns) *ns = &dummy;
    return GENX_SUCCESS;
}

genxStatus genxStartAttributeLiteral(genxWriter w, constUtf8 xmlns, constUtf8 name) {
    return GENX_SUCCESS;
}

genxStatus genxEndAttribute(genxWriter w) {
    return GENX_SUCCESS;
}

genxStatus genxAddAttributeLiteral(genxWriter w, constUtf8 xmlns, constUtf8 name, constUtf8 value) {
    return GENX_SUCCESS;
}

genxStatus genxAddCountedText(genxWriter w, constUtf8 start, int byteCount) {
    return GENX_SUCCESS;
}

void genxSetUserData(genxWriter w, void *userData) {
}

void genxSetPrettyPrint(genxWriter w, int flags) {
}
