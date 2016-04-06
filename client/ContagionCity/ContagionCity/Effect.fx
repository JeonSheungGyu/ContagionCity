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

cbuffer cbSkinned
{
	// �� �ɸ����� �ִ� ���� ���� 96��
	float4x4 gBoneTransforms[96];
};

//t�� �ؽ���
//s�� ���÷�
Texture2D gtxtTexture : register(t0);
SamplerState gSamplerState : register(s0);

Texture2D gtxtNormalTexture : register( t1 );
SamplerState gNormalSamplerState : register( s1 );

//������
Texture2D gtxtDetailTexture : register(t2);
SamplerState gDetailSamplerState : register(s2);


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

//�ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	//float4 tangent : TANGENT;
	//float3 weights : WEIGHTS;
	//uint4 boneIndices; BONEINDICES;
};

//�ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD0;
};

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
	input.normalW = gtxtNormalTexture.Sample( gSamplerState, input.texCoord );

	float4 cIllumination = Lighting(input.positionW, input.normalW);
		float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord) * cIllumination;

		return(cColor);
}


struct SkinnedVertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexCoord : TEXCOORD;
	float4 TangentL : TANGENT;
	float3 Weights : WEIGHTS;
	float4 BoneIndices : BONEINDICES;
};

struct SkinnedVertexOut
{
	float4 PosH       : SV_POSITION;
	float3 PosW       : POSITION;
	float3 NormalW    : NORMAL;
	float4 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD0;
};

//SkinnedVertexOut SkinnedVS( SkinnedVertexIn vin )
//{
//	SkinnedVertexOut vout;
//
//	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
//	weights[0] = vin.weights.x;
//	weights[1] = vin.weights.y;
//	weights[2] = vin.weights.z;
//	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];
//
//	float3 posL = float3( 0.0f, 0.0f, 0.0f );
//		float3 normalL = float3( 0.0f, 0.0f, 0.0f );
//		float3 tangentL = float3( 0.0f, 0.0f, 0.0f );
//		for (int i = 0; i < 4; ++i)
//		{
//			posL += weights[i] * mul( float4( vin.posL, 1.0f ), gBoneTransforms[vin.BoneIndices[i]] ).xyz;
//			normalL += weights[[i] * mul( vin.normalL, ( float3x3 )gBoneTransforms[vin.BoneIndices[i]] );
//			tangentL += weights[i] * mul( vin.tangentL.xyz, ( float3x3 )gBoneTransforms[vin.BoneIndices[i]] );
//		}
//
//	vout.PosW = mul( float4x4( posL, 1.0f ), gmtxWorld ).xyz;
//	vout.TangentW = float4( mul( tangentL, ( float3x3 )gmtxWorld ), vin.tangentL.w );
//	vout.PosH = mul( float4( posL, 1.0f ), gmtxWorld );
//	vout.PosH = mul( vout.PosH, gmtxView );
//	vout.PosH = mul( vout.PosH, gmtxProjection );
//
//	vout.Tex = vin.Tex;
//
//	return vout;
//}
//
//float4 SkinnedPS( SkinnedVertexOut input ) : SV_Target
//{
//	input.normalW = gtxtNormalTexture.Sample( gSamplerState, input.Tex );
//
//	float4 cIllumination = Lighting( input.PosW, input.NormalW );
//		float4 cColor = gtxtTexture.Sample( gSamplerState, input.Tex ) * cIllumination;
//
//		return( cColor );
//}