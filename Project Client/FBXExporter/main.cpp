#include"etc.h"
#include"FBXExporter.h"

int main()
{
	FBXExporter *pFBXExporter = new FBXExporter();
	pFBXExporter->Initialize();
	pFBXExporter->LoadScene("./test.fbx");
	pFBXExporter->ExportFBX();

	int nVertices = pFBXExporter->GetVertices();
	int nIndices = pFBXExporter->GetIndices();

	printf("V : %d, I : %d\n", nVertices, nIndices);

	return 0;
}