const int MAX_LIGHTS = 8;
const int TYPE_POINT = 0;
const int TYPE_DIRECTIONAL = 1;
const int TYPE_SPOT = 2;
struct Light {
    int type;
    FP vec3 position;
    FP vec3 color;
    FP float intensity;
    FP vec3 direction;
    FP vec3 attenuation;
//    FP float cutOffAngle;
};
uniform Light lights[MAX_LIGHTS];
uniform int lightCount;

void adsModelNormalMapped(const in FP vec3 vpos, const in FP vec3 vnormal, const in FP vec3 eye, const in FP float shininess,
                          const in FP mat3 tangentMatrix,
                          out FP vec3 diffuseColor, out FP vec3 specularColor)
{
    diffuseColor = vec3(0.0);
    specularColor = vec3(0.0);

    FP vec3 n = normalize( vnormal );

    int i;
    FP vec3 s;
    for (i = 0; i < lightCount; ++i) {
        FP float att = 1.0;
        if ( lights[i].type != TYPE_DIRECTIONAL ) {
            s = lights[i].position - vpos;
            if (length( lights[i].attenuation ) != 0.0) {
                FP float dist = length(s);
                att = 1.0 / (lights[i].attenuation.x + lights[i].attenuation.y * dist + lights[i].attenuation.z * dist * dist);
            }
        } else {
            s = -lights[i].direction;
        }

        s = normalize( tangentMatrix * s );
        FP float diffuse = max( dot( s, n ), 0.0 );

        FP float specular = 0.0;
        if (diffuse > 0.0 && shininess > 0.0) {
            FP vec3 r = reflect( -s, n );
            FP vec3 v = normalize( tangentMatrix * ( eye - vpos ) );
            FP float normFactor = ( shininess + 2.0 ) / 2.0;
            specular = normFactor * pow( max( dot( r, v ), 0.0 ), shininess );
        }

        diffuseColor += att * lights[i].intensity * diffuse * lights[i].color;
        specularColor += specular;
    }
}

void adsModel(const in FP vec3 vpos, const in FP vec3 vnormal, const in FP vec3 eye, const in FP float shininess,
              out FP vec3 diffuseColor, out FP vec3 specularColor)
{
    diffuseColor = vec3(0.0);
    specularColor = vec3(0.0);

    FP vec3 n = normalize( vnormal );

    int i;
    FP vec3 s;
    for (i = 0; i < lightCount; ++i) {
        FP float att = 1.0;
        if ( lights[i].type != TYPE_DIRECTIONAL ) {
            s = lights[i].position - vpos;
            if (length( lights[i].attenuation ) != 0.0) {
                FP float dist = length(s);
                att = 1.0 / (lights[i].attenuation.x + lights[i].attenuation.y * dist + lights[i].attenuation.z * dist * dist);
            }
        } else {
            s = -lights[i].direction;
        }

        s = normalize( s );
        FP float diffuse = max( dot( s, n ), 0.0 );

        FP float specular = 0.0;
        if (diffuse > 0.0 && shininess > 0.0) {
            FP vec3 r = reflect( -s, n );
            FP vec3 v = normalize( eye - vpos );
            FP float normFactor = ( shininess + 2.0 ) / 2.0;
            specular = normFactor * pow( max( dot( r, v ), 0.0 ), shininess );
        }

        diffuseColor += att * lights[i].intensity * diffuse * lights[i].color;
        specularColor += specular;
    }
}

void adModel(const in FP vec3 vpos, const in FP vec3 vnormal, out FP vec3 diffuseColor)
{
    diffuseColor = vec3(0.0);

    FP vec3 n = normalize( vnormal );

    int i;
    FP vec3 s;
    for (i = 0; i < lightCount; ++i) {
        FP float att = 1.0;
        if ( lights[i].type != TYPE_DIRECTIONAL ) {
            s = lights[i].position - vpos;
            if (length( lights[i].attenuation ) != 0.0) {
                FP float dist = length(s);
                att = 1.0 / (lights[i].attenuation.x + lights[i].attenuation.y * dist + lights[i].attenuation.z * dist * dist);
            }
        } else {
            s = -lights[i].direction;
        }

        s = normalize( s );
        FP float diffuse = max( dot( s, n ), 0.0 );

        diffuseColor += att * lights[i].intensity * diffuse * lights[i].color;
    }
}