#include "Light.fx"

//��������
//����
cbuffer cbViewProjectionMatrix : register(b0)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
};

cbuffer cbWorldMatrix : register(b1)
{
	matrix gmtxWorld : packoffset(c0);
};

//t�� �ؽ���
//s�� ���÷�
Texture2D gtxtTexture : register(t0);
SamplerState gSamplerState : register(s0);
//������
Texture2D gtxtDetailTexture : register(t1);
SamplerState gDetailSamplerState : register(s1);



//����ü
struct VS_INPUT
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};



//������ ������ ���� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_DIFFUSED_COLOR_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR0;
};

//������ ������ ���� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_DIFFUSED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

//�ν��Ͻ��� �ϸ鼭 ������ ������ ���� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_DIFFUSED_COLOR_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR0;
	float4x4 mtxTransform : INSTANCEPOS;
};

//�ν��Ͻ��� �ϸ鼭 ������ ������ ���� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_DIFFUSED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

//������ ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

//������ ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	//������ǥ�迡�� ������ ��ġ�� ���� ���͸� ��Ÿ����.
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

//�ν��Ͻ��� �ϸ鼭 ������ ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float4x4 mtxTransform : INSTANCEPOS;
};

//�ν��Ͻ��� �ϸ鼭 ������ ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

//�ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_TEXTURED_COLOR_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
};

//�ؽ��ĸ� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_TEXTURED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

//�ν��Ͻ��� �ϸ鼭 �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_COLOR_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float4x4 mtxTransform : INSTANCEPOS;
};

//�ν��Ͻ��� �ϸ鼭 �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

//������ �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է°� ����� ���� ����ü�̴�.
struct VS_DETAIL_TEXTURED_COLOR_INPUT
{
	float3 position : POSITION;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

struct VS_DETAIL_TEXTURED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};




/*����-���̴��̴�.
������ ��ġ ���͸� ���� ��ȯ, ī�޶� ��ȯ, ���� ��ȯ�� ������� �����Ѵ�.
���� �ﰢ���� �� ������ y-�������� ȸ���� ��Ÿ���� ��Ŀ� ���� ��ȯ�Ѵ�.
�׷��Ƿ� �ﰢ���� ȸ���ϰ� �ȴ�.
*/
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.position = mul(input.position, gmtxWorld);
	output.position = mul(output.position, gmtxView);
	output.position = mul(output.position, gmtxProjection);
	output.color = input.color;
	//�ԷµǴ� ������ ������ �״�� ����Ѵ�. 
	return output;
}
// �ȼ�-���̴�
float4 PS(VS_OUTPUT input) : SV_Target
{
	return input.color;
	//�ԷµǴ� ������ ������ �״�� ����Ѵ�. 
}

////////////////////////////////////////

struct VS_INSTANCED_COLOR_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR0;
	column_major float4x4 mtxTransform : INSTANCEPOS;
	float4 instanceColor : INSTANCECOLOR;
};

struct VS_INSTANCED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	//�ý��� ���� ������ ���� ���̴��� ���޵Ǵ� ��ü �ν��Ͻ��� ID�� �ȼ� ���̴��� �����Ѵ�.
	float4 instanceID : INDEX;
};

///

//�ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

//�ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD0;
};

//������ �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_DETAIL_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

//������ �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

//�ν��Ͻ�, �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4x4 mtxTransform : INSTANCEPOS;
};

//�ν��Ͻ�, �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD0;
};///


VS_INSTANCED_COLOR_OUTPUT VSInstancedDiffusedColor(VS_INSTANCED_COLOR_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_INSTANCED_COLOR_OUTPUT output = (VS_INSTANCED_COLOR_OUTPUT)0;
	output.position = mul(float4(input.position, 1), input.mtxTransform);
	output.position = mul(output.position, gmtxView);
	output.position = mul(output.position, gmtxProjection);
	output.color = (instanceID % 5) ? input.color : (input.color * 0.3f + input.instanceColor * 0.7f);
	output.instanceID = instanceID;
	return output;
}

float4 PSInstancedDiffusedColor(VS_INSTANCED_COLOR_OUTPUT input) : SV_Target
{
	return input.color;
}



VS_DIFFUSED_COLOR_OUTPUT VSDiffusedColor(VS_DIFFUSED_COLOR_INPUT input)
{
	VS_DIFFUSED_COLOR_OUTPUT output = (VS_DIFFUSED_COLOR_OUTPUT)0;
	output.position = mul(float4(input.position, 1.0f), mul(mul(gmtxWorld, gmtxView), gmtxProjection));
	output.color = input.color;

	return(output);
}

float4 PSDiffusedColor(VS_DIFFUSED_COLOR_OUTPUT input) : SV_Target
{
	return(input.color);
}

VS_INSTANCED_DIFFUSED_COLOR_OUTPUT VSInstancedDiffusedColor(VS_INSTANCED_DIFFUSED_COLOR_INPUT input)
{
	VS_INSTANCED_DIFFUSED_COLOR_OUTPUT output = (VS_INSTANCED_DIFFUSED_COLOR_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), input.mtxTransform), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSInstancedDiffusedColor(VS_INSTANCED_DIFFUSED_COLOR_OUTPUT input) : SV_Target
{
	return(input.color);
}

//������ ������ ����ϱ� ���� ������ ���� ���Ϳ� ������ ��ġ�� ����ϴ� ���� ���̴� �Լ��̴�.
VS_LIGHTING_COLOR_OUTPUT VSLightingColor(VS_LIGHTING_COLOR_INPUT input)
{
	VS_LIGHTING_COLOR_OUTPUT output = (VS_LIGHTING_COLOR_OUTPUT)0;
	//������ ������ ����ϱ� ���Ͽ� ������ǥ�迡�� ������ ��ġ�� ���� ���͸� ���Ѵ�.
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}

//�� �ȼ��� ���Ͽ� ������ ������ �ݿ��� ������ ����ϱ� ���� �ȼ� ���̴� �Լ��̴�.
float4 PSLightingColor(VS_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);

		return(cIllumination);
}


VS_INSTANCED_LIGHTING_COLOR_OUTPUT VSInstancedLightingColor(VS_INSTANCED_LIGHTING_COLOR_INPUT input)
{
	VS_INSTANCED_LIGHTING_COLOR_OUTPUT output = (VS_INSTANCED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)input.mtxTransform);
	output.positionW = mul(float4(input.position, 1.0f), input.mtxTransform).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}

float4 PSInstancedLightingColor(VS_INSTANCED_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);

		return(cIllumination);
}


//�ؽ���

VS_TEXTURED_COLOR_OUTPUT VSTexturedColor(VS_TEXTURED_COLOR_INPUT input)
{
	VS_TEXTURED_COLOR_OUTPUT output = (VS_TEXTURED_COLOR_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;

	return(output);
}

//�� �ȼ��� ���Ͽ� �ؽ��� ���ø��� �ϱ� ���� �ȼ� ���̴� �Լ��̴�.
float4 PSTexturedColor(VS_TEXTURED_COLOR_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord);

	return(cColor);
}

//�ν��Ͻ� _ �ؽ���

VS_INSTANCED_TEXTURED_COLOR_OUTPUT VSInstancedTexturedColor(VS_INSTANCED_TEXTURED_COLOR_INPUT input)
{
	VS_INSTANCED_TEXTURED_COLOR_OUTPUT output = (VS_INSTANCED_TEXTURED_COLOR_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), input.mtxTransform), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;

	return(output);
}

float4 PSInstancedTexturedColor(VS_INSTANCED_TEXTURED_COLOR_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord);

	return(cColor);
}


///������ �ؽ���
VS_DETAIL_TEXTURED_COLOR_OUTPUT VSDetailTexturedColor(VS_DETAIL_TEXTURED_COLOR_INPUT input)
{
	VS_DETAIL_TEXTURED_COLOR_OUTPUT output = (VS_DETAIL_TEXTURED_COLOR_OUTPUT)0;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.texCoordBase = input.texCoordBase;
	output.texCoordDetail = input.texCoordDetail;

	return(output);
}

float4 PSDetailTexturedColor(VS_DETAIL_TEXTURED_COLOR_OUTPUT input) : SV_Target
{
	float4 cBaseTexColor = gtxtTexture.Sample(gSamplerState, input.texCoordBase);
	float4 cDetailTexColor = gtxtDetailTexture.Sample(gDetailSamplerState, input.texCoordDetail);
	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor);
}


VS_TEXTURED_LIGHTING_COLOR_OUTPUT VSTexturedLightingColor(VS_TEXTURED_LIGHTING_COLOR_INPUT input)
{
	VS_TEXTURED_LIGHTING_COLOR_OUTPUT output = (VS_TEXTURED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;

	return(output);
}

float4 PSTexturedLightingColor(VS_TEXTURED_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
		float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord) * cIllumination;

		return(cColor);
}
VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT VSDetailTexturedLightingColor(VS_DETAIL_TEXTURED_LIGHTING_COLOR_INPUT input)
{
	VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT output = (VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.texCoordBase = input.texCoordBase;
	output.texCoordDetail = input.texCoordDetail;

	return(output);
}

float4 PSDetailTexturedLightingColor(VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
		float4 cBaseTexColor = gtxtTexture.Sample(gSamplerState, input.texCoordBase);
		float4 cDetailTexColor = gtxtDetailTexture.Sample(gDetailSamplerState, input.texCoordDetail);
		float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

		return(cColor*cIllumination);
}


VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT VSInstancedTexturedLightingColor(VS_INSTANCED_TEXTURED_LIGHTING_COLOR_INPUT input)
{
	VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT output = (VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)input.mtxTransform);
	output.positionW = mul(float4(input.position, 1.0f), input.mtxTransform).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.texCoord = input.texCoord;

	return(output);
}

float4 PSInstancedTexturedLightingColor(VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
		float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord) * cIllumination;

		return(cColor);
}
