using UnityEngine;
using UnityEditor;
using System.IO;
using System.Linq;
using UnityEditor.Formats.Fbx.Exporter;

public class BulkFBXExporter
{
    private static string ExportPath = "Assets/ExportedFBX";

    [MenuItem("Assets/Bulk FBX Export...", false)]
    public static void Export()
    {
        string folderPath = AssetDatabase.GetAssetPath(Selection.activeObject);

        if (string.IsNullOrEmpty(folderPath) || !Directory.Exists(folderPath))
        {
            Debug.LogError("Please select a folder to export to");
            return;
        }

        Debug.Log("Exporting to " + folderPath);

        string[] guids = AssetDatabase.FindAssets("t:Prefab", new string[] { folderPath });
        if (guids.Length == 0)
        {
            Debug.LogError("No prefabs found in " + folderPath);
            return;
        }

        foreach (string guid in guids)
        {
            string assetPath = AssetDatabase.GUIDToAssetPath(guid);
            GameObject prefab = AssetDatabase.LoadAssetAtPath<GameObject>(assetPath);
            ExportInternal(prefab);
        }

        // refresh the asset database so that the file appears in the
        // asset folder view.
        AssetDatabase.Refresh();
    }

    private static void ExportInternal(GameObject target)
    {
        string filePath = Path.Combine(ExportPath, target.name + ".fbx");

        if (System.IO.File.Exists(filePath))
        {
            Debug.LogError("File already exists: " + filePath);
            return;
        }

        ExportModelOptions options = new ExportModelOptions();

        options.ExportFormat = ExportFormat.Binary;
        options.LODExportType = LODExportType.Highest;
        options.EmbedTextures = false;
        options.ObjectPosition = ObjectPosition.WorldAbsolute;
        options.ModelAnimIncludeOption = Include.Model;
        options.ExportUnrendered = true;
        options.KeepInstances = true;

        if (ModelExporter.ExportObject(filePath, target, options) == null)
        {
            Debug.LogError("Failed to export " + target.name);
        }

        Debug.Log("Exported " + target.name + " to " + filePath);
    }

    [MenuItem("Assets/Bulk FBX Export...", true)]
    private static bool ValidatePrintFolderPath()
    {
        string path = AssetDatabase.GetAssetPath(Selection.activeObject);
        return !string.IsNullOrEmpty(path) && Directory.Exists(path);
    }
}
