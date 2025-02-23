using UnityEngine;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

public class TerrainExporter : MonoBehaviour
{
    private struct TreeInstanceData
    {
        public float position_x, position_y, position_z;
        public float size_width, size_height;
        public float rotation_y;
        public int prototype_index;
    }

    private struct TreePrototypeData
    {
        public string mesh_name;
        public string[] texture_names;
    }

    [SerializeField]
    private Terrain terrain;

    [SerializeField]
    private string exportName;

    public void Export()
    {
        TerrainData terrainData = terrain.terrainData;
        
        var treeInstances = terrainData.treeInstances;
        var treePrototypes = terrainData.treePrototypes;
        List<TreeInstanceData> treeInstancesList = new List<TreeInstanceData>();
        for (var i = 0; i < treeInstances.Length; i++)
        {
            var treeInstance = treeInstances[i];
            // remove the "Prefab_" prefix
            treeInstancesList.Add(new TreeInstanceData()
            {
                position_x = treeInstance.position.x,
                position_y = treeInstance.position.y,
                position_z = treeInstance.position.z,
                size_width = treeInstance.widthScale,
                size_height = treeInstance.heightScale,
                rotation_y = treeInstance.rotation,
                prototype_index = treeInstance.prototypeIndex
            });
        }

        string treeInstancesJson = JsonSerializer.Serialize(treeInstancesList, new JsonSerializerOptions() { IncludeFields = true, IgnoreReadOnlyProperties = true });
        string treeInstancesPath = Application.dataPath + '\\' + exportName + "_treeInstances.json";
        File.WriteAllText(treeInstancesPath, treeInstancesJson);

        Debug.Log("Exported terrain to " + treeInstancesPath);
    }

    private void Start()
    {
        Export();
    }
}
