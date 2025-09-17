using UnityEngine;
using System.IO;

public class GrassMapExporter : MonoBehaviour
{
    public Terrain terrain;
    public int layerIndex = 0; // Change if you want another detail layer

    void Start()
    {
        ExportGrassMap();
    }

    void ExportGrassMap()
    {
        TerrainData terrainData = terrain.terrainData;
        int width = terrainData.detailWidth;
        int height = terrainData.detailHeight;

        int[,] detailLayer = terrainData.GetDetailLayer(0, 0, width, height, layerIndex);
        Texture2D texture = new Texture2D(width, height, TextureFormat.RGB24, false);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float value = detailLayer[x, y] / 16.0f; // Normalize (assuming max density of 16)
                texture.SetPixel(x, y, new Color(value, value, value));
            }
        }

        texture.Apply();

        byte[] bytes = texture.EncodeToPNG();
        File.WriteAllBytes(Application.dataPath + "/GrassMap.png", bytes);
        Debug.Log("Grass map saved at: " + Application.dataPath + "/GrassMap.png");
    }
}